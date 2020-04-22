import sys
from db1_allFunctions import db1_insert_all, db1_read_all, db1_update_all, db1_delete_all
from db2_allFunctions import db2_insert_all, db2_read_all, db2_update_all, db2_delete_all
from market_allFunctions import market_insert_all, market_read_all, market_read_specific, market_update_all, market_delete_all

def readUserInfo():
  #prompt user to enter user info
  #initializes name variable with a large array to prevent overflow
  name = "\0" * 100 
  name = raw_input("Enter username: ")

  #prompt user to enter password and quit program if incorrect
  #initializes password variable with a large array to prevent overflow
  password = "\0" * 100
  password = raw_input("Enter password: ")

  #matches user info to the password
  if name == "Scott" and password == "capstone1":
    print ("\nWelcome " + name)
  
  elif name == "Andrew" and password == "capstone2":
    print ("\nWelcome " + name)

  elif name == "Felecia" and password == "capstone3":
    print ("\nWelcome " + name)
    
  elif name == "Pete" and password == "capstone4":
    print ("\nWelcome " + name)

  #if user/password combo does not match, then the program exits
  else:
    print ("\nIncorrect username and/or password. \nPlease try again.")
    quit()

  return name

def adminData():
  adminMessage = "\nAdmins: Please work diligently, but keep is simple.\n"

  #Message for Scott
  adminScott = ["Admins: Hello, Scott! Have fun creating!"]

  #Message for Andrew
  adminAndrew = ["Admins: Hello, Andrew! Have fun reading!"]
  
  #Message for Felecia
  adminFelecia = ["Admins: Hello, Felecia! Have fun updating!"]
  
  #Message for Pete
  adminPete = ["Admins: Hello, Pete! Have fun deleting!"]

  name = readUserInfo()
  
  #Create functions for Scott
  if name == "Scott":
    print (adminScott)
    dbChoice = raw_input("Please type in the name of the database you would like to add to: \ndb1 or db2? \n")
    if dbChoice == "db1":
      db1_insert_all()
    elif dbChoice == "db2":
      db2_insert_all()
    else:
      print ("Please enter a valid database.")
    print("You've successfully created this document.")

  #Read function for Andrew
  if name == "Andrew":
    print (adminAndrew)
    dbChoice = raw_input("Please type in the name of the database you would like to read from: \nmarket, db1, or db2? \n")
    if dbChoice == "market":
      market_read_specific()
    elif dbChoice == "db1":
      db1_read_all()
    elif dbChoice == "db2":
      db2_read_all()
    else:
      print ("Please enter a valid database.")
    print ("You've successfully read this document.")  
    
  #Update functions for Felecia
  if name == "Felecia":
    print (adminFelecia)
    dbChoice = raw_input("Please type in the name of the database you would like to update: \ndb1 or db2? \n")
    if dbChoice == "db1":
      db1_update_all()
    elif dbChoice == "db2":
      db2_update_all()
    else:
      print ("Please enter a valid database entry.")
    print ("You've successfully updated this document.")

  #Delete functions for Pete
  if name == "Pete":
    print (adminPete)
    dbChoice = raw_input("Please type in the name of the database you would like to delete from: \ndb1 or db2? \n")
    if dbChoice == "db1":
      db1_delete_all()
    elif dbChoice == "db2":
      db2_delete_all()
    else:
      print ("Please enter a valid database.")
    print ("You've successfully deleted this document.")
    
  print (adminMessage)

adminData()