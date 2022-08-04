var BOT_ACCESS_TOKEN = '*****';
var SPREADSHEET_ID = '*****';
var userProperties = PropertiesService.getUserProperties();
  
function doPost(e) {

  var SpreadSheet = SpreadsheetApp.openById(SPREADSHEET_ID);
  var Sheet = SpreadSheet.getSheets()[0];
  var reply_message;

  if (e.postData) {

    var msg = JSON.parse(e.postData.contents);
    const userMessage = msg.events[0].message.text.trim();
    const user_id = msg.events[0].source.userId;
    const event_type = msg.events[0].source.type;
    const replyToken = msg.events[0].replyToken;

    try {
      userProperties.setProperty('replyToken', replyToken);
    } catch (err) {
      Logger.log('Failed with error %s', err.message);
    }

    if (userMessage=="help") {

      reply_message = [{
            "type": "text",
            "text": "Command list",
            "quickReply": {
                "items": [
                    {
                        "type": "action",
                        "action": {
                            "type": "message",
                            "label": "on",
                            "text": "on"
                        }
                    },
	                  {
                        "type": "action",
                        "action": {
                            "type": "message",
                            "label": "off",
                            "text": "off"
                        }
                    }
                ]
            }
      }] 

      sendMessageToLineBot(BOT_ACCESS_TOKEN,replyToken,reply_message);           
    }
    else {
      Sheet.getRange(1,1).setValue(userMessage);
      Sheet.getRange(1,2).setValue(replyToken);
    }

  }
  else if (e.parameter.response) {

    if (e.parameter.token == userProperties.getProperty('replyToken')) {
      reply_message = [{
        "type":"text",
        "text": e.parameter.response
      }]
      Sheet.getRange(1,1).setValue("");
      Sheet.getRange(1,2).setValue("");

      sendMessageToLineBot(BOT_ACCESS_TOKEN, e.parameter.token, reply_message);
    }   

  }

  return  ContentService.createTextOutput("Return = Finish");
}

function sendMessageToLineBot(accessToken, replyToken, reply_message) {

  var url = 'https://api.line.me/v2/bot/message/reply';
  UrlFetchApp.fetch(url, {
    'headers': {
      'Content-Type': 'application/json; charset=UTF-8',
      'Authorization': 'Bearer ' + accessToken,
    },
    'method': 'post',
    'payload': JSON.stringify({
      'replyToken': replyToken,
      'messages': reply_message
    }),
  });
  
} 