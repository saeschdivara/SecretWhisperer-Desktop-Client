
_class_instance_ChatController = null

chatApp.controller('ChatController', ['$rootScope', '$scope', '$sce', 'MessageQueue', ($rootScope, $scope, $sce, MessageQueue) ->

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

        constructor: (message, from_me, from_contact, is_file) ->
            ###
            ###

            @DATA_TYPES =
                images: ['jpg', 'jpeg', 'png', 'gif']
                audios: ['mp3']
                movies: ['mp4', 'webm']

            @message = message
            @is_from_me = from_me
            @is_from_contact = from_contact

            if @is_from_me
                @message_classes = ['my-message', 'blue', 'lighten-3']
            else if @is_from_contact
                @message_classes = ['partner-message', 'blue', 'darken-1']

            if is_file
                @message_type = @getType()
            else
                @message_type = 'text'


        getType: () ->
            ###
            ###

            if @_isType('images', @message)
                'image'
            else if @_isType('audios', @message)
                'audio'
            else if @_isType('movies', @message)
                'movie'
            else
                'downloadable'


        link: () ->
            ###
            ###

            @message


        image: () ->
            ###
            ###

            @message


        audio: () ->
            ###
            ###

            $sce.trustAsResourceUrl(@message)


        movie: () ->
            ###
            ###

            $sce.trustAsResourceUrl(@message)


        text: () ->
            ###
            ###

            @message

        _isType: (type_name, data_string) ->
            ###
            ###

            for type in @DATA_TYPES[type_name]
                if data_string.toLowerCase().indexOf("." + type) > -1
                    return true

            return false


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

            chat.sendMessageToUser(@current_contact.username, message)
            create_message_from_me(message)


        onUploadFile: () =>
            ###
            ###

            $file_input = jQuery('#file-input-button')

            $file_input.change(
                () =>
                    console.log('file chosen')

                    reader = new FileReader()
                    reader.onload = () =>
                        url = reader.result
                        console.log(url)
                        chat.sendMessageToUser(@current_contact.username, url)

                    files = $file_input[0].files
                    reader.readAsDataURL(files[0])
            )

            $file_input.click()


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
                    new ChatMessage(message_text, false, true, false)
                )
            else
                for contact in @contacts
                    if contact.username == username
                        contact.messages.push(new ChatMessage(message_text, false, true, false))

            $scope.$apply()


        onFileFromContact: (message) =>
            ###
            ###

            username = message.contact
            message_text = message.message

            if @current_contact.username == username
                @current_messages.push(
                    new ChatMessage(message_text, false, true, true)
                )
            else
                for contact in @contacts
                    if contact.username == username
                        contact.messages.push(new ChatMessage(message_text, false, true, true))

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
    MessageQueue.$subscribe('new-file-from-other', controller.onFileFromContact)


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