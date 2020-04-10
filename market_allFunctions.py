import json
from bson import json_util
from pymongo import MongoClient
import datetime
import pymongo
import re
import sys

# Connecting to MongoHost
try:
  connection = MongoClient('localhost',27017)
  print("Connected to MongoHost Successfully!")
except:
  print("Could not connect to MongoHost")

# database name
db=connection['market']

# collection name
collection=db['stocks']

def market_insert_all():
  # Function to Insert document with specific key/value pairs into collection
  def insert_document(document):
      while True:
          try:
              result = collection.insert_one(document)
              print("Data inserted into collection successfully")

          except ValueError:
              print("Data insert unsuccessful")
              
          return result

  def mainInsert():
      myDocument = {"business_name": "Capstone",
                    "date": datetime.datetime.utcnow(),
                    "result": "CS 499",
                    "sector": "Newest Retail",
                    "Comments": "New management"}

      print (insert_document(myDocument))
      print("true")
      
  mainInsert()

def market_read_all():
  # Read document with key/value pairs in the collection  
  def read_document(self):
    try:
      cursor = collection.find({ "business_name" : "Capstone", 
                               "result" : "CS 499" })
      for myDocument in cursor:
        print(myDocument)
        print("Data read successfully")
        
    except ValidationError as ve:
      print("Data read unsuccessfully")
      abort(400,str(ve))
      
    return myDocument
  
  def mainRead1():
      myDocument = {"business_name": "Capstone",
                    "date": datetime.datetime.utcnow(),
                    "result": "CS 499",
                    "sector": "Newest Retail",
                    "Comments": "New management"}
      
      print("All documents from myDBtest collection")
      print read_document(myDocument)
      
  mainRead()

def market_read_specific():
  myDocument = ""
  
  # Read documents with specific key/value pairs in the collection  
  def read_document(self):
    
    myDocument = [collection.find()]
    fieldName = raw_input("Enter the field you want to filter through: \n")
    valueName = raw_input("Enter the specific value you want to search for: \n")
    
    #Searches the entire collection for the specified information
    myDocument = collection.find({ fieldName : { '$regex' : valueName }}, { fieldName : 1, "_id" : 0 }) 

    #Returns the desired results
    for item in myDocument:
      print(item)
      
    return myDocument
  
  def mainRead2():
      myDocument = [collection.find()]
      
      print("All documents from myDBtest collection")
      read_document(myDocument)
      
  mainRead2()
  
def market_update_all():
  # Function to update document with a new key/value pair
  def update_document(self):
      while True:
          try:
              query = {"business_name": "Capstone"}
              new_myDocument = {"$set": {"business_name": "CapstoneNew"}}
              collection.update_many(query, new_myDocument)

              for myDocument in collection.find({"business_name": "CapstoneNew"}):
                  print("Data updated successfully")
                  print(myDocument)

          except ValidationError as ve:
              print("Data update unsuccessful")
              abort(400, str(ve))

          return myDocument

  def mainUpdate():
      myDocument = {"business_name": "CapstoneNew",
                    "date": datetime.datetime.utcnow(),
                    "result": "CS 499",
                    "sector": "Newest Retail",
                    "Comments": "New management"}

      print
      update_document(myDocument)
      print("true")

  mainUpdate()
  
def market_delete_all():
  # Delete document with key/value pairs in the collection
  def delete_document(self):
    try:
      delete_docs = collection.delete_many({ "business_name" : "CapstoneNew" })
      print(delete_docs.deleted_count, " articles deleted successfully")
      
    except ValidationError as ve:
      print("Data deleted unsuccessfully")
      abort(400,str(ve))
      
    return
  
  def mainDelete():
    myDocument = {"business_name": "Capstone",
                  "date": datetime.datetime.utcnow(),
                  "result": "CS 499",
                  "sector": "Newest Retail",
                  "Comments": "New management"}
    
    print delete_document(myDocument)
      
  mainDelete()  
      