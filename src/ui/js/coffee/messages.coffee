
create_message_from_me = (message) ->
    _class_instance_MessageQueue.$publish('new-message-from-me', message)


create_message_to_me = (message) ->
    _class_instance_MessageQueue.$publish('new-message-from-other', message)


create_file_message_to_me = (message) ->
    _class_instance_MessageQueue.$publish('new-file-from-other', message)