

onDocumentReady = () ->

    ############################
    ## MESSAGES CALLBACKS
    ###########################


    onUserMessageReceived = (username, message) ->
        create_message_to_me(
            contact: username
            message: message
        )


    #############################
    ## USER CALLBACKS
    #############################

    onUserChosen = () ->

        username = jQuery('#other-username').val()
        chat.connectToUser(username)

        jQuery('#chatbox').show()


    onUserAdded = (username) ->
        _class_instance_MessageQueue.$publish('add-contact', username)
        jQuery('#chatbox').show()


    onServerConnect = () ->
        console.log('Connected')

        jQuery('#submit-user-button').click(
            () ->

                console.log('on login button clicked')

                my_user_name = jQuery('#username').val()

                chat.chooseUserName(my_user_name)

                jQuery('#userbox').hide()

                jQuery('#submit-other-user-button').click(onUserChosen)

                chat.connectionToUserEstablished.connect(onUserAdded)
                chat.receivedUserMessage.connect(onUserMessageReceived)
        )

    onServerError = (error) ->
        console.log(error)


    chat.connected.connect(onServerConnect)
    chat.error.connect(onServerError)
    chat.connectToServer('localhost', 8888)

    console.log('connected to the server')

    jQuery('#chatbox').hide()



jQuery(document).ready(onDocumentReady)

create_message_from_me = (message) ->
    _class_instance_MessageQueue.$publish('new-message-from-me', message)


create_message_to_me = (message) ->
    _class_instance_MessageQueue.$publish('new-message-from-other', message)
chatApp = angular.module('chatApp', [])

_class_instance_MessageQueue = null
 
chatApp.factory('MessageQueue', [() ->
 
    class MessageQueue
     
        ########################
        ## PRIVATE PROPERTIES ##
        ########################

        _channels: {}
     
     
        #######################
        ## PUBLIC PROPERTIES ##
        #######################
     
     
        ####################
        ## PUBLIC METHODS ##
        ####################
     
        constructor: () ->
            ###
            ###


        $subscribe: (channel, subscriber) ->
            ###
            ###


            if not @_channels.hasOwnProperty(channel)
                @_channels[channel] = []

            @_channels[channel].push(subscriber)


        $publish: (channel, message) ->
            ###
            ###

            if @_channels.hasOwnProperty(channel)
                for subscriber in @_channels[channel]
                    subscriber(message)

     
     
        @$instance: () ->
            ###
            ###
     
            if not _class_instance_MessageQueue?
                # Create instance
                _class_instance_MessageQueue = new MessageQueue()
     
            return _class_instance_MessageQueue
     
        #####################
        ## PRIVATE METHODS ##
        #####################
 
    # Return the factory
    MessageQueue.$instance()
])


log = (message) ->
    jQuery('#message-board').append("<span>#{message}</span>")


getRandomHexString = () ->
    ###
     Generates random hex number string

     @return {String}
    ###
    return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1)

_class_instance_ChatController = null

chatApp.controller('ChatController', ['$rootScope', '$scope', 'MessageQueue', ($rootScope, $scope, MessageQueue) ->

    class ChatMessage
        ########################
        ## PRIVATE PROPERTIES ##
        ########################

        #######################
        ## PUBLIC PROPERTIES ##
        #######################

        ####################
        ## PUBLIC METHODS ##
        ####################

        constructor: (message, from_me, from_contact) ->
            ###
            ###

            @message = message
            @is_from_me = from_me
            @is_from_contact = from_contact


        text: () ->
            ###
            ###

            @message


    class ChatContact
        ########################
        ## PRIVATE PROPERTIES ##
        ########################

        #######################
        ## PUBLIC PROPERTIES ##
        #######################

        ####################
        ## PUBLIC METHODS ##
        ####################

        constructor: (@username) ->
            ###
            ###

            @messages = []
            @active = true



    ################################################################
    # STATIC CONTROLLER CLASS
    ################################################################
    class ChatController
        ########################
        ## PRIVATE PROPERTIES ##
        ########################

        #######################
        ## PUBLIC PROPERTIES ##
        #######################

        contacts: []
        current_contact: null
        current_messages: []
        current_typed_message: ''

        ####################
        ## PUBLIC METHODS ##
        ####################

        constructor: () ->
            ###
            ###

            @contacts = []
            @current_messages = []

            @is_file_chosen = false


        onContactChanged: (contact) ->
            ###
            ###

            if @current_contact is contact
                return

            @current_contact.messages.length = 0
            @current_contact.messages.push(message) for message in @current_messages
            @current_contact.active = false

            @current_contact = contact

            @current_messages.length = 0
            @current_messages.push(message) for message in contact.messages
            @current_contact.active = true


        onMessageSend: () =>
            ###
            ###

            message = @current_typed_message
            @current_typed_message.length = 0
            @current_typed_message = ''

            if not @is_file_chosen
                chat.sendMessageToUser(@current_contact.username, message)
            else
                reader = new FileReader()
                reader.onload = () =>
                    url = reader.result
                    console.log(url)
                    chat.sendMessageToUser(@current_contact.username, url)

                files = jQuery('#file-input-button')[0].files
                reader.readAsDataURL(files[0])

                @is_file_chosen = false

            create_message_from_me(message)


        onUploadFile: () =>
            ###
            ###

            jQuery('#file-input-button').click()
            @is_file_chosen = true


        onMessageFromMe: (message) =>
            ###
            ###

            @current_messages.push(
                new ChatMessage(message, true, false)
            )

            $scope.$apply()


        onMessageFromContact: (message) =>
            ###
            ###

            username = message.contact
            message_text = message.message

            if @current_contact.username == username
                @current_messages.push(
                    new ChatMessage(message_text, false, true)
                )
            else
                for contact in @contacts
                    if contact.username == username
                        contact.messages.push(new ChatMessage(message_text, false, true))

            $scope.$apply()


        onContactAdded: (username) =>
            ###
            ###

            for contact in @contacts
                if contact.username == username
                    return
                else
                    contact.active = false

            contact = new ChatContact(username)
            @contacts.push(contact)

            @current_contact = contact

            $scope.$apply()


        @$instance: () ->
            ###
            ###
            if not _class_instance_ChatController?
                # Create instance
                _class_instance_ChatController = new ChatController()
            return _class_instance_ChatController

        #####################
        ## PRIVATE METHODS ##
        #####################

    #######################
    ## CREATE CONTROLLER ##
    #######################

    controller = ChatController.$instance()

    ################################################################
    # CONFIGURE CONTROLLER
    ################################################################

    MessageQueue.$subscribe('add-contact', controller.onContactAdded)

    MessageQueue.$subscribe('new-message-from-me', controller.onMessageFromMe)
    MessageQueue.$subscribe('new-message-from-other', controller.onMessageFromContact)


    ################################################################
    # CONTROLLER SCOPE
    ################################################################

    ################
    ## INIT SCOPE ##
    ################

    #################
    ## WATCH SCOPE ##
    #################

    ################################################################
    # RETURN CONTROLLER
    ################################################################

    return controller
])