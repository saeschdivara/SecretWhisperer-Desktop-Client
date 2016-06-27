(function() {
  var _class_instance_ChatController, _class_instance_MessageQueue, chatApp, create_file_message_to_me, create_message_from_me, create_message_to_me, getRandomHexString, log, mainFunction, onDocumentReady, startUpWithWebChannel,
    bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

  startUpWithWebChannel = function() {
    return new QWebChannel(qt.webChannelTransport, function(channel) {
      return mainFunction(channel.objects.chat);
    });
  };

  mainFunction = function(chat) {
    var onContactsLoaded, onServerConnect, onServerError, onUserAdded, onUserChosen, onUserFileReceived, onUserMessageReceived;
    window.chat = chat;
    onUserMessageReceived = function(username, message) {
      return create_message_to_me({
        contact: username,
        message: message
      });
    };
    onUserFileReceived = function(username, message) {
      return create_file_message_to_me({
        contact: username,
        message: message
      });
    };
    onUserChosen = function() {
      var username;
      username = jQuery('#other-username').val();
      chat.connectToUser(username);
      return jQuery('#chatbox').show();
    };
    onUserAdded = function(username) {
      _class_instance_MessageQueue.$publish('add-contact', username);
      return jQuery('#chatbox').show();
    };
    onContactsLoaded = function(contactsString) {
      var contact, contacts, i, len, results;
      contacts = contactsString.split('|');
      results = [];
      for (i = 0, len = contacts.length; i < len; i++) {
        contact = contacts[i];
        results.push(onUserAdded(contact));
      }
      return results;
    };
    onServerConnect = function() {
      console.log('Connected');
      return jQuery('#submit-user-button').click(function() {
        var my_user_name, my_user_password;
        console.log('on login button clicked');
        my_user_name = jQuery('#username').val();
        my_user_password = jQuery('#password').val();
        chat.chooseUserName(my_user_name, my_user_password);
        jQuery('#userbox').hide();
        jQuery('#submit-other-user-button').click(onUserChosen);
        chat.connectionToUserEstablished.connect(onUserAdded);
        chat.contactsLoaded.connect(onContactsLoaded);
        chat.receivedUserMessage.connect(onUserMessageReceived);
        chat.receivedUserFile.connect(onUserFileReceived);
        return chat.loadContacts();
      });
    };
    onServerError = function(error) {
      return console.log(error);
    };
    chat.connected.connect(onServerConnect);
    chat.error.connect(onServerError);
    chat.connectToServer(window.CHAT_SERVER_URL, 8888);
    console.log('connected to the server');
    return jQuery('#chatbox').hide();
  };

  onDocumentReady = function() {
    return startUpWithWebChannel();
  };

  jQuery(document).ready(onDocumentReady);

  create_message_from_me = function(message) {
    return _class_instance_MessageQueue.$publish('new-message-from-me', message);
  };

  create_message_to_me = function(message) {
    return _class_instance_MessageQueue.$publish('new-message-from-other', message);
  };

  create_file_message_to_me = function(message) {
    return _class_instance_MessageQueue.$publish('new-file-from-other', message);
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
        function ChatMessage(message, from_me, from_contact, is_file) {

          /*
           */
          this.message = message;
          this.is_from_me = from_me;
          this.is_from_contact = from_contact;
          if (this.is_from_me) {
            this.message_classes = ['my-message', 'blue', 'lighten-3'];
          } else if (this.is_from_contact) {
            this.message_classes = ['partner-message', 'blue', 'darken-1'];
          }
          if (is_file) {
            this.message_type = this.getType();
          } else {
            this.message_type = 'text';
          }
        }

        ChatMessage.prototype.getType = function() {

          /*
           */
          if (this.message.indexOf('.jpg') > -1 || this.message.indexOf('.png') > -1) {
            return 'image';
          } else {
            return 'downloadable';
          }
        };

        ChatMessage.prototype.link = function() {

          /*
           */
          return this.message;
        };

        ChatMessage.prototype.image = function() {

          /*
           */
          return this.message;
        };

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
          this.messages = [];
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
          this.onFileFromContact = bind(this.onFileFromContact, this);
          this.onMessageFromContact = bind(this.onMessageFromContact, this);
          this.onMessageFromMe = bind(this.onMessageFromMe, this);
          this.onUploadFile = bind(this.onUploadFile, this);
          this.onMessageSend = bind(this.onMessageSend, this);

          /*
           */
          this.contacts = [];
          this.current_messages = [];
          this.is_file_chosen = false;
        }

        ChatController.prototype.onContactChanged = function(contact) {

          /*
           */
          var i, j, len, len1, message, ref, ref1;
          if (this.current_contact === contact) {
            return;
          }
          this.current_contact.messages.length = 0;
          ref = this.current_messages;
          for (i = 0, len = ref.length; i < len; i++) {
            message = ref[i];
            this.current_contact.messages.push(message);
          }
          this.current_contact.active = false;
          this.current_contact = contact;
          this.current_messages.length = 0;
          ref1 = contact.messages;
          for (j = 0, len1 = ref1.length; j < len1; j++) {
            message = ref1[j];
            this.current_messages.push(message);
          }
          return this.current_contact.active = true;
        };

        ChatController.prototype.onMessageSend = function() {

          /*
           */
          var files, message, reader;
          message = this.current_typed_message;
          this.current_typed_message.length = 0;
          this.current_typed_message = '';
          if (!this.is_file_chosen) {
            chat.sendMessageToUser(this.current_contact.username, message);
          } else {
            reader = new FileReader();
            reader.onload = (function(_this) {
              return function() {
                var url;
                url = reader.result;
                console.log(url);
                return chat.sendMessageToUser(_this.current_contact.username, url);
              };
            })(this);
            files = jQuery('#file-input-button')[0].files;
            reader.readAsDataURL(files[0]);
            this.is_file_chosen = false;
          }
          return create_message_from_me(message);
        };

        ChatController.prototype.onUploadFile = function() {

          /*
           */
          jQuery('#file-input-button').click();
          return this.is_file_chosen = true;
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
          var contact, i, len, message_text, ref, username;
          username = message.contact;
          message_text = message.message;
          if (this.current_contact.username === username) {
            this.current_messages.push(new ChatMessage(message_text, false, true, false));
          } else {
            ref = this.contacts;
            for (i = 0, len = ref.length; i < len; i++) {
              contact = ref[i];
              if (contact.username === username) {
                contact.messages.push(new ChatMessage(message_text, false, true, false));
              }
            }
          }
          return $scope.$apply();
        };

        ChatController.prototype.onFileFromContact = function(message) {

          /*
           */
          var contact, i, len, message_text, ref, username;
          username = message.contact;
          message_text = message.message;
          if (this.current_contact.username === username) {
            this.current_messages.push(new ChatMessage(message_text, false, true, true));
          } else {
            ref = this.contacts;
            for (i = 0, len = ref.length; i < len; i++) {
              contact = ref[i];
              if (contact.username === username) {
                contact.messages.push(new ChatMessage(message_text, false, true, true));
              }
            }
          }
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
      MessageQueue.$subscribe('new-file-from-other', controller.onFileFromContact);
      return controller;
    }
  ]);

}).call(this);

//# sourceMappingURL=app.js.map
