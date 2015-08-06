
onServerConnect = () ->
    console.log('Connected')

    jQuery('#submit-button').click(
        () ->
            chat.chooseUserName(jQuery('#username').text())
            chat.connectToUser('other_user')
    )

onServerError = (error) ->
    console.log(error)


chat.connected.connect(onServerConnect)
chat.error.connect(onServerError)
chat.connectToServer('localhost', 8888)