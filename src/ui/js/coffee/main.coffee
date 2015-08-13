

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