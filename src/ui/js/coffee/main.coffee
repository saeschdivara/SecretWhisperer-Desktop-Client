
startUpWithWebChannel = () ->

    new QWebChannel(qt.webChannelTransport, (channel) ->
        mainFunction(channel.objects.chat)
    )

mainFunction = (chat) ->

    # Expose chat as global object
    window.chat = chat

    ############################
    ## MESSAGES CALLBACKS
    ###########################

    onUserMessageReceived = (username, message) ->
        create_message_to_me(
            contact: username
            message: message
        )

    onUserFileReceived = (username, message) ->
        create_file_message_to_me(
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
                my_user_password = jQuery('#password').val()

                chat.chooseUserName(my_user_name, my_user_password)

                jQuery('#userbox').hide()
                jQuery('#submit-other-user-button').click(onUserChosen)

                chat.connectionToUserEstablished.connect(onUserAdded)
                chat.receivedUserMessage.connect(onUserMessageReceived)
                chat.receivedUserFile.connect(onUserFileReceived)
        )

    onServerError = (error) ->
        console.log(error)


    chat.connected.connect(onServerConnect)
    chat.error.connect(onServerError)
    chat.connectToServer(window.CHAT_SERVER_URL, 8888)

    console.log('connected to the server')

    jQuery('#chatbox').hide()

onDocumentReady = () ->
    # Starting first to connect to web channel
    startUpWithWebChannel()

jQuery(document).ready(onDocumentReady)