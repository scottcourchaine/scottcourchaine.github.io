#!/usr/bin/python
import json
from bson import json_util
import bottle
from bottle import route, run, post, put, delete, request, abort
import datetime
import requests
import Capstone_Functions as db
  
# Function to Create Document with specific key/values into collection  
@route('/createStock', method='POST')
def get_createStock():
  try:
    data = json.load(request.body)
    
    myDocument={"id" : data['id'], 
                "Ticker" : data['Ticker'],
                "Volume" : data['Volume']}
    
    db.insert_document(myDocument)
    
  except NameError:
      abort(404, 'Not Found!')
      
# Function to Read Document with specific key/value lookup pairs in collection      
@route('/getStock', method='GET')
def get_readStock():
  try:
    Ticker=request.query.Ticker
    if Ticker:
      page = db.read_document(Ticker)
      
      if page is None:
        return 'Not Found'
      
    else:
      return 'Ticker cant be null'  
      
  except NameError:
      abort(404, 'Not Found!')
      
  return json.loads(json.dumps(page, indent=4, default=json_util.default))
      
# Function to Read Document with specific key/value lookup pairs in collection      
@route('/stockReport', method='GET')
def get_readStock():
  try:
    Ticker=request.query.Ticker
    
    if Ticker:
      tickers=Ticker.split(',')
      pages = []
      for ticker in tickers:
        if ticker:
          page = db.read_document(ticker)
          
          if page is None:
            pages.append('Not Found') 
          else:
            pages.append(page)
      
    else:
      return 'Ticker cant be null'  
      
  except NameError:
      abort(404, 'Not Found!')
  data = ""
  for page in pages:
    data += json.dumps(page, indent=4, default=json_util.default)
    
  return data

# Function to Read Document with specific key/value lookup pairs in collection      
@route('/industryReport', method='GET')
def get_industryReport():
  try:
    Industry=request.query.Industry
    if Industry:
      cursor = db.read_Industry(Industry)
      print(cursor.count())
      
      if cursor is None:
        return 'Not Found'
      
      context = {}
      i = 0
      while i<5 and cursor.alive:
        page = cursor.next()

        context[i] = page
        i+=1

      cursor.close()
      
    else:
      return 'Industry cant be null'  
      
  except NameError:
      abort(404, 'Not Found!')
      
  # This print is for the RESTful terminal to see DESCENDING Order in proper form of Index 0 - 4
  print (json.dumps(context, indent=4, default=json_util.default))    
  
  return json.loads(json.dumps(context, indent=4, default=json_util.default))

# Function to Read Document with specific key/value lookup pairs in collection      
@route('/indusTicker', method='GET')
def get_indusTicker():
  try:
    Industry=request.query.Industry
    if Industry:
      cursor = db.read_Industry(Industry)
      print(cursor.count())
      
      print(cursor.count())
      if cursor is None:
        return 'Not Found'
      
      context = []
      for page in cursor:
        context.append(page["Ticker"])
        print(page)
      
    else:
      return 'Industry cant be null'  
      
  except NameError:
      abort(404, 'Not Found!')    
  
  data = ""
  
  for ticker in context:
    data += json.dumps(ticker, indent=4, default=json_util.default)
    
  return data
    
# Function to Update Document with specific key/value lookup pairs in collection      
@route('/updateStock', method='GET')
def get_updateStock():
  try:
    dbTicker=request.query.Ticker
    Volume=request.query.Volume
    print(dbTicker)
    print(Volume)
    if dbTicker is None or Volume is None:
      return 'Ticker & Volume cant be null'
      
    db.update_document(dbTicker, Volume)
    
  except NameError:
    abort(404, 'Not Found!')

# Function to Delete Document with specific key/value lookup pairs in collection
@route('/deleteStock', method='GET')
def delete_stock():
  try:
    dbTicker=request.query.Ticker
    if dbTicker is None:
      return 'Ticker cant be null'

    db.delete_document(dbTicker)
    
  except NameError:
    abort(404, 'Not Found!')
    
# Function to Read Document with specific key/value lookup pairs in collection      
@route('/getAve', method='GET')
def get_readAve():
  try:
    page = db.read_docu()

    print('okfound)')
    if page is None:
      return 'Not Found' 
      
  except NameError:
      abort(404, 'Not Found!')
      
  return json.dumps(page, indent=4, default=json_util.default)
    
if __name__ == '__main__':
  #app.run(debug=True)
  run(host='localhost', port=8080)