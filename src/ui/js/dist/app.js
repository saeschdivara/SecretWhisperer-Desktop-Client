(function() {
  var getRandomHexString, log, onDocumentReady;

  onDocumentReady = function() {
    var onServerConnect, onServerError, onUserConnected, onUserMessageSend, other_user_name;
    other_user_name = '';
    onUserMessageSend = function() {
      return chat.sendMessageToUser(other_user_name, jQuery('chat-input').val());
    };
    onUserConnected = function() {
      jQuery('#chatbox').show();
      return jQuery('#submit-message-button').click(onUserMessageSend);
    };
    onServerConnect = function() {
      console.log('Connected');
      return jQuery('#submit-button').click(function() {
        chat.chooseUserName(jQuery('#username').text());
        jQuery('#userbox').hide();
        onUserConnected();
        other_user_name = jQuery('#other-username').text();
        return chat.connectToUser(other_user_name);
      });
    };
    onServerError = function(error) {
      return console.log(error);
    };
    chat.connected.connect(onServerConnect);
    chat.error.connect(onServerError);
    chat.connectToServer('localhost', 8888);
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
