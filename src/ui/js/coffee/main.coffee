

onDocumentReady = () ->

    my_user_name = ''
    other_user_name = ''

    user_list = []


    create_message_from_me = (message) ->
        jQuery("
            <div class='message-box my-message blue lighten-3'>
                <div class='message'>
                    <p>#{message}</p>
                </div>
            </div>
        ")


    create_message_to_me = (message) ->
        jQuery("
            <div class='message-box partner-message blue darken-1'>
                <div class='message'>
                    <p>#{message}</p>
                </div>
            </div>
        ")


    onUserMessageSend = () ->
        console.log('sending message')

        message = jQuery('#chat-input').val()
        jQuery('#chat-input').val("")
        chat.sendMessageToUser(other_user_name, message)


        created_message = create_message_from_me(message)

        jQuery("#chat-messages").append(created_message)


    onUserMessageReceived = (username, message) ->
        console.log('Name: ' + username)
        console.log("Message: #{message}")


        created_message = create_message_to_me(message)
        jQuery("#chat-messages").append(created_message)


    onUserConnected = () ->

        jQuery('#otheruserbox').show()
        jQuery('#submit-other-user-button').click(onUserChosen)
        jQuery('#cancel-other-user-button').click(onCancelUserChosen)


    onUserChosen = () ->

        other_user_name = jQuery('#other-username').val()
        chat.connectToUser(other_user_name)

        jQuery('#otheruserbox').hide()
        jQuery('#chatbox').show()
        jQuery('#submit-message-button').click(onUserMessageSend)

        chat.receivedUserMessage.connect(onUserMessageReceived)


    onCancelUserChosen = () ->

        jQuery('#otheruserbox').hide()
        jQuery('#chatbox').show()
        jQuery('#submit-message-button').click(onUserMessageSend)

        chat.receivedUserMessage.connect(onUserMessageReceived)


    onUserAdded = (username) ->
        console.log("User added: #{username}")
        user_list.push(username)

        other_user_name = username


    onServerConnect = () ->
        console.log('Connected')

        jQuery('#submit-user-button').click(
            () ->

                console.log('on login button clicked')

                my_user_name = jQuery('#username').val()

                chat.chooseUserName(my_user_name)

                jQuery('#userbox').hide()

                onUserConnected()

                chat.connectionToUserEstablished.connect(onUserAdded)
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