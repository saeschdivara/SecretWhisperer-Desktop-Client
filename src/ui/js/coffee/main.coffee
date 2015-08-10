

onDocumentReady = () ->

    my_user_name = ''
    other_user_name = ''


    onUserMessageSend = () ->
        console.log('sending message')

        message = jQuery('#chat-input').val()
        chat.sendMessageToUser(other_user_name, message)


    onUserConnected = () ->

        jQuery('#otheruserbox').show()
        jQuery('#submit-other-user-button').click(onUserChosen)


    onUserMessageReceived = (username, message) ->
        console.log('Name: ' + username)
        console.log("Message: #{message}")


    onUserChosen = () ->

        other_user_name = jQuery('#other-username').val()
        chat.connectToUser(other_user_name)

        jQuery('#otheruserbox').hide()
        jQuery('#chatbox').show()
        jQuery('#submit-message-button').click(onUserMessageSend)

        chat.receivedUserMessage.connect(onUserMessageReceived)


    onServerConnect = () ->
        console.log('Connected')

        jQuery('#submit-user-button').click(
            () ->

                console.log('on login button clicked')

                my_user_name = jQuery('#username').val()

                chat.chooseUserName(my_user_name)

                jQuery('#userbox').hide()

                onUserConnected()
        )

    onServerError = (error) ->
        console.log(error)


    chat.connected.connect(onServerConnect)
    chat.error.connect(onServerError)
    chat.connectToServer('localhost', 8888)

    console.log('connected to the server')

    jQuery('#chatbox').hide()
    jQuery('#otheruserbox').hide()




jQuery(document).ready(onDocumentReady)