(function() {
  window.log = function(message) {
    return jQuery('#message-board').append("<span>" + message + "</span>");
  };

}).call(this);

//# sourceMappingURL=app.js.map
