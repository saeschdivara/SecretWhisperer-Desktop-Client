(function() {
  var onServerConnect, onServerError;

  onServerConnect = function() {
    console.log('Connected');
    return chat.connectToUser('other_user');
  };

  onServerError = function(error) {
    return console.log(error);
  };

  chat.connected.connect(onServerConnect);

  chat.error.connect(onServerError);

  chat.connectToServer('localhost', 8888);

  window.log = function(message) {
    return jQuery('#message-board').append("<span>" + message + "</span>");
  };

}).call(this);

//# sourceMappingURL=app.js.map
