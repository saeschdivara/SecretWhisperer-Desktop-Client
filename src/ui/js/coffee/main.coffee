

onDocumentReady = () ->

    my_user_name = ''
    other_user_name = ''

    ############################
    ## MESSAGES CALLBACKS
    ###########################


    onUserMessageReceived = (username, message) ->
        create_message_to_me(message)


    #############################
    ## USER CALLBACKS
    #############################

    onUserChosen = () ->

        other_user_name = jQuery('#other-username').val()
        chat.connectToUser(other_user_name)

        _class_instance_MessageQueue.$publish('add-contact', other_user_name)

        jQuery('#chatbox').show()


    onUserAdded = (username) ->
        console.log("User added: #{username}")
        other_user_name = username
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