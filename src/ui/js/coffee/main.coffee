

onDocumentReady = () ->

    other_user_name = ''


    onUserMessageSend = () ->
        chat.sendMessageToUser(other_user_name, jQuery('chat-input').val())


    onUserConnected = () ->

        jQuery('#chatbox').show()

        jQuery('#submit-message-button').click(onUserMessageSend)


    onServerConnect = () ->
        console.log('Connected')

        jQuery('#submit-button').click(
            () ->
                chat.chooseUserName(jQuery('#username').text())

                jQuery('#userbox').hide()

                onUserConnected()

                other_user_name = jQuery('#other-username').text()
                chat.connectToUser(other_user_name)
        )

    onServerError = (error) ->
        console.log(error)


    chat.connected.connect(onServerConnect)
    chat.error.connect(onServerError)
    chat.connectToServer('localhost', 8888)

    jQuery('#chatbox').hide()




jQuery(document).ready(onDocumentReady)