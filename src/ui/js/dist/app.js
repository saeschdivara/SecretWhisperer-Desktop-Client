(function() {
  var getRandomHexString, log, onDocumentReady;

  onDocumentReady = function() {
    var my_user_name, onServerConnect, onServerError, onUserConnected, onUserMessageSend, other_user_name;
    my_user_name = '';
    other_user_name = '';
    onUserMessageSend = function() {
      var message;
      console.log('sending message');
      message = jQuery('#chat-input').val();
      return chat.sendMessageToUser(other_user_name, message);
    };
    onUserConnected = function() {
      jQuery('#chatbox').show();
      return jQuery('#submit-message-button').click(onUserMessageSend);
    };
    onServerConnect = function() {
      console.log('Connected');
      return jQuery('#submit-user-button').click(function() {
        console.log('on login button clicked');
        my_user_name = jQuery('#username').val();
        other_user_name = jQuery('#other-username').val();
        chat.chooseUserName(my_user_name);
        jQuery('#userbox').hide();
        onUserConnected();
        return chat.connectToUser(other_user_name);
      });
    };
    onServerError = function(error) {
      return console.log(error);
    };
    chat.connected.connect(onServerConnect);
    chat.error.connect(onServerError);
    chat.connectToServer('localhost', 8888);
    console.log('connected to the server');
    return jQuery('#chatbox').hide();
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
