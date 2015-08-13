
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


        onMessageSend: () ->
            ###
            ###

            message = @current_typed_message
            @current_typed_message.length = 0
            @current_typed_message = ''

            chat.sendMessageToUser(@current_contact.username, message)

            create_message_from_me(message)

            $scope.$apply()



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