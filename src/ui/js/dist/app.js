(function() {
  var _class_instance_ChatController, _class_instance_MessageQueue, chatApp, create_message_from_me, create_message_to_me, getRandomHexString, log, onDocumentReady,
    bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

  onDocumentReady = function() {
    var my_user_name, onServerConnect, onServerError, onUserAdded, onUserChosen, onUserMessageReceived, other_user_name;
    my_user_name = '';
    other_user_name = '';
    onUserMessageReceived = function(username, message) {
      return create_message_to_me(message);
    };
    onUserChosen = function() {
      other_user_name = jQuery('#other-username').val();
      chat.connectToUser(other_user_name);
      _class_instance_MessageQueue.$publish('add-contact', other_user_name);
      return jQuery('#chatbox').show();
    };
    onUserAdded = function(username) {
      console.log("User added: " + username);
      other_user_name = username;
      _class_instance_MessageQueue.$publish('add-contact', username);
      return jQuery('#chatbox').show();
    };
    onServerConnect = function() {
      console.log('Connected');
      return jQuery('#submit-user-button').click(function() {
        console.log('on login button clicked');
        my_user_name = jQuery('#username').val();
        chat.chooseUserName(my_user_name);
        jQuery('#userbox').hide();
        jQuery('#submit-other-user-button').click(onUserChosen);
        chat.connectionToUserEstablished.connect(onUserAdded);
        return chat.receivedUserMessage.connect(onUserMessageReceived);
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

  create_message_from_me = function(message) {
    return _class_instance_MessageQueue.$publish('new-message-from-me', message);
  };

  create_message_to_me = function(message) {
    return _class_instance_MessageQueue.$publish('new-message-from-other', message);
  };

  chatApp = angular.module('chatApp', []);

  _class_instance_MessageQueue = null;

  chatApp.factory('MessageQueue', [
    function() {
      var MessageQueue;
      MessageQueue = (function() {
        MessageQueue.prototype._channels = {};

        function MessageQueue() {

          /*
           */
        }

        MessageQueue.prototype.$subscribe = function(channel, subscriber) {

          /*
           */
          if (!this._channels.hasOwnProperty(channel)) {
            this._channels[channel] = [];
          }
          return this._channels[channel].push(subscriber);
        };

        MessageQueue.prototype.$publish = function(channel, message) {

          /*
           */
          var i, len, ref, results, subscriber;
          if (this._channels.hasOwnProperty(channel)) {
            ref = this._channels[channel];
            results = [];
            for (i = 0, len = ref.length; i < len; i++) {
              subscriber = ref[i];
              results.push(subscriber(message));
            }
            return results;
          }
        };

        MessageQueue.$instance = function() {

          /*
           */
          if (_class_instance_MessageQueue == null) {
            _class_instance_MessageQueue = new MessageQueue();
          }
          return _class_instance_MessageQueue;
        };

        return MessageQueue;

      })();
      return MessageQueue.$instance();
    }
  ]);

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

  _class_instance_ChatController = null;

  chatApp.controller('ChatController', [
    '$rootScope', '$scope', 'MessageQueue', function($rootScope, $scope, MessageQueue) {
      var ChatContact, ChatController, ChatMessage, controller;
      ChatMessage = (function() {
        function ChatMessage(message, from_me, from_contact) {

          /*
           */
          this.message = message;
          this.is_from_me = from_me;
          this.is_from_contact = from_contact;
        }

        ChatMessage.prototype.text = function() {

          /*
           */
          return this.message;
        };

        return ChatMessage;

      })();
      ChatContact = (function() {
        function ChatContact(username1) {
          this.username = username1;

          /*
           */
          this.active = true;
        }

        return ChatContact;

      })();
      ChatController = (function() {
        ChatController.prototype.contacts = [];

        ChatController.prototype.current_contact = null;

        ChatController.prototype.current_messages = [];

        ChatController.prototype.current_typed_message = '';

        function ChatController() {
          this.onContactAdded = bind(this.onContactAdded, this);
          this.onMessageFromContact = bind(this.onMessageFromContact, this);
          this.onMessageFromMe = bind(this.onMessageFromMe, this);

          /*
           */
          this.contacts = [];
          this.current_messages = [];
        }

        ChatController.prototype.onMessageSend = function() {

          /*
           */
          var message;
          message = this.current_typed_message;
          this.current_typed_message.length = 0;
          chat.sendMessageToUser(this.current_contact.username, message);
          create_message_from_me(message);
          return $scope.$apply();
        };

        ChatController.prototype.onMessageFromMe = function(message) {

          /*
           */
          this.current_messages.push(new ChatMessage(message, true, false));
          return $scope.$apply();
        };

        ChatController.prototype.onMessageFromContact = function(message) {

          /*
           */
          this.current_messages.push(new ChatMessage(message, false, true));
          return $scope.$apply();
        };

        ChatController.prototype.onContactAdded = function(username) {

          /*
           */
          var contact, i, len, ref;
          ref = this.contacts;
          for (i = 0, len = ref.length; i < len; i++) {
            contact = ref[i];
            if (contact.username === username) {
              return;
            } else {
              contact.active = false;
            }
          }
          contact = new ChatContact(username);
          this.contacts.push(contact);
          this.current_contact = contact;
          return $scope.$apply();
        };

        ChatController.$instance = function() {

          /*
           */
          if (_class_instance_ChatController == null) {
            _class_instance_ChatController = new ChatController();
          }
          return _class_instance_ChatController;
        };

        return ChatController;

      })();
      controller = ChatController.$instance();
      MessageQueue.$subscribe('add-contact', controller.onContactAdded);
      MessageQueue.$subscribe('new-message-from-me', controller.onMessageFromMe);
      MessageQueue.$subscribe('new-message-from-other', controller.onMessageFromContact);
      return controller;
    }
  ]);

}).call(this);

//# sourceMappingURL=app.js.map
