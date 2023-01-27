import serial
import mysql.connector

from datetime import datetime


def tratamentoDadosbd(string):
    # retira o inicio
    string = string.replace("b'","")
    # retira o final
    tirar = string.find('\\')
    string = string[:tirar]

    return string




# informações para conexão com o banco de dados
mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  password="", # insira a senha do banco de dados
  database = "arduino"
)
# cursor do banco de dados
mycursor = mydb.cursor()


# teste de conexão serial
try:
  portaSerial = serial.Serial('/dev/ttyUSB0', 9600, timeout = 2) # no windows -> COM
  portaSerial.isOpen()
  print ("conectado")
except serial.SerialException:
  print("Porta USB não detectada")




'''
  Aguarda informações da porta serial
'''
recebeuIdTag = False
while (not recebeuIdTag):
  # recebe dados do arduino
  serialValor = str(portaSerial.readline())

  # tratamento de dados
  data_arduino = tratamentoDadosbd(serialValor)
  

  '''
    Porta serial recebeu informação id tag
  '''
  if (data_arduino[0] != ''):

    # enviar para função .execute como lista
    enviarBd = list(data_arduino)
    
    # consulta da tag no banco de dados
    mycursor.execute(f"SELECT canecas,cod,nome FROM clientes WHERE id = '{enviarBd[0]}'")
    myresult = mycursor.fetchone()

    # condição de saída do loop
    recebeuIdTag = True



'''
  Envia quantidade de canacas para o arduino
'''
portaSerial.write( myresult[:1])          # envio da quantidade de canecas
enviarNome = myresult[2].encode('utf-8') 
portaSerial.write( enviarNome )           #envio do nome


'''
  Atualiza a quant de canecas no bd (desconta uma caneca)
'''
mycursor.execute(f"UPDATE clientes SET canecas = {myresult[0]-1} WHERE cod = {myresult[1]}")
mycursor.execute(f"INSERT INTO consumo (cod_cliente, data_consumo) values ({myresult[1]}, '{datetime.now()}') " )
mydb.commit()



'''
  Fechamento da comunicação com banco de dados
'''
mycursor.close()
mydb.close()
print("Comunicação com banco de dados e arduino encerrada !")




