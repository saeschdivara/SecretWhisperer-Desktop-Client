(function() {
  var getRandomHexString, log, onDocumentReady;

  onDocumentReady = function() {
    var create_message_from_me, create_message_to_me, my_user_name, onCancelUserChosen, onServerConnect, onServerError, onUserAdded, onUserChosen, onUserConnected, onUserMessageReceived, onUserMessageSend, other_user_name, user_list;
    my_user_name = '';
    other_user_name = '';
    user_list = [];
    create_message_from_me = function(message) {
      return jQuery("<div class='message-box my-message blue lighten-3'> <div class='message'> <p>" + message + "</p> </div> </div>");
    };
    create_message_to_me = function(message) {
      return jQuery("<div class='message-box partner-message blue darken-1'> <div class='message'> <p>" + message + "</p> </div> </div>");
    };
    onUserMessageSend = function() {
      var created_message, message;
      console.log('sending message');
      message = jQuery('#chat-input').val();
      jQuery('#chat-input').val("");
      chat.sendMessageToUser(other_user_name, message);
      created_message = create_message_from_me(message);
      return jQuery("#chat-messages").append(created_message);
    };
    onUserMessageReceived = function(username, message) {
      var created_message;
      console.log('Name: ' + username);
      console.log("Message: " + message);
      created_message = create_message_to_me(message);
      return jQuery("#chat-messages").append(created_message);
    };
    onUserConnected = function() {
      jQuery('#otheruserbox').show();
      jQuery('#submit-other-user-button').click(onUserChosen);
      return jQuery('#cancel-other-user-button').click(onCancelUserChosen);
    };
    onUserChosen = function() {
      other_user_name = jQuery('#other-username').val();
      chat.connectToUser(other_user_name);
      jQuery('#otheruserbox').hide();
      jQuery('#chatbox').show();
      jQuery('#submit-message-button').click(onUserMessageSend);
      return chat.receivedUserMessage.connect(onUserMessageReceived);
    };
    onCancelUserChosen = function() {
      jQuery('#otheruserbox').hide();
      jQuery('#chatbox').show();
      jQuery('#submit-message-button').click(onUserMessageSend);
      return chat.receivedUserMessage.connect(onUserMessageReceived);
    };
    onUserAdded = function(username) {
      console.log("User added: " + username);
      user_list.push(username);
      return other_user_name = username;
    };
    onServerConnect = function() {
      console.log('Connected');
      return jQuery('#submit-user-button').click(function() {
        console.log('on login button clicked');
        my_user_name = jQuery('#username').val();
        chat.chooseUserName(my_user_name);
        jQuery('#userbox').hide();
        onUserConnected();
        return chat.connectionToUserEstablished.connect(onUserAdded);
      });
    };
    onServerError = function(error) {
      return console.log(error);
    };
    chat.connected.connect(onServerConnect);
    chat.error.connect(onServerError);
    chat.connectToServer('localhost', 8888);
    console.log('connected to the server');
    jQuery('#chatbox').hide();
    return jQuery('#otheruserbox').hide();
  };

  jQuery(document).ready(onDocumentReady);

  log = function(message) {
    return jQuery('#message-board').append("<span>" + message + "</span>");
  };

  getRandomHexString = function() {

    /*
     Generates random hex number string
    
     @return {String}
     */
    return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
  };

}).call(this);

//# sourceMappingURL=app.js.map
