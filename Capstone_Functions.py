import json
from bson import json_util
from pymongo import MongoClient
import datetime 
import pymongo

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

# Function to Insert document with specific key/value pairs into collection
def insert_document(document):
      try:
        result=collection.insert_one(document)
        print ("Data inserted into stocks collection successfully")

      except ValidationError as ve:
        print ("Data insert unsuccessful")
        abort(400,str(ve))
      return result
      
# Function to Read Document with specific key/value lookup pairs in collection
def read_document(Ticker):
      try:
        myDocument = collection.find_one({"Ticker" : Ticker})

      except ValidationError as ve:
        print ("Data read unsuccessful")
        abort(400,str(ve))
      return myDocument  
    
# Function to Read Document with specific key/value lookup pairs in collection
def read_name(Industry):
      try:
        myDocument = collection.find_one({"Industry" : Industry})

      except ValidationError as ve:
        print ("Data read unsuccessful")
        abort(400,str(ve))
      return myDocument
    
# Function to Read Document with specific key/value lookup pairs in collection
def read_Industry(Industry):
      try:
        myResult = collection.find({"Industry" : Industry}).sort([("Industry", pymongo.DESCENDING)])
      
      except ValidationError as ve:
        print ("Data read unsuccessful")
        abort(400,str(ve))
      return myResult
    
# Function to Read Document with specific key/value lookup pairs in collection
def read_docu():
      try:
        myResult = collection.find_one({'50-Day Simple Moving Average' : {"$exists": True}})
      
      except ValidationError as ve:
        print ("Data read unsuccessful")
        abort(400,str(ve))
      return myResult
      
# Function to Update Document with specific key/value lookup pairs in collection
def update_document(Ticker, Volume):
      try:
        query = {"Ticker": Ticker}
        new_myDocument = {"$set": {"Volume": Volume}}

        collection.update_many(query, new_myDocument)
        
      except ValidationError as ve:
        print ("Data update unsuccessful")
        abort(400,str(ve))
      return new_myDocument
      
# Function to Delete Document with specific key/value lookup pairs in collection
def delete_document(Ticker):
      try:
        delete_docs = collection.delete_many({"Ticker":Ticker})

        print(delete_docs.deleted_count, " Documents deleted successfully.")

      except ValidationError as ve:
        print ("Data delete unsuccessful")
        abort(400,str(ve))
      return 