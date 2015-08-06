(function() {
  var getRandomHexString, log, onServerConnect, onServerError;

  onServerConnect = function() {
    console.log('Connected');
    return jQuery('#submit-button').click(function() {
      chat.chooseUserName(jQuery('#username').text());
      return chat.connectToUser('other_user');
    });
  };

  onServerError = function(error) {
    return console.log(error);
  };

  chat.connected.connect(onServerConnect);

  chat.error.connect(onServerError);

  chat.connectToServer('localhost', 8888);

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
