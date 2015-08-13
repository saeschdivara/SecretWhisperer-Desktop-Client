
_class_instance_MessageQueue = null
 
chatApp.factory('MessageQueue', [() ->
 
    class MessageQueue
     
        ########################
        ## PRIVATE PROPERTIES ##
        ########################

        _channels: {}
     
     
        #######################
        ## PUBLIC PROPERTIES ##
        #######################
     
     
        ####################
        ## PUBLIC METHODS ##
        ####################
     
        constructor: () ->
            ###
            ###


        $subscribe: (channel, subscriber) ->
            ###
            ###


            if not @_channels.hasOwnProperty(channel)
                @_channels[channel] = []

            @_channels[channel].push(subscriber)


        $publish: (channel, message) ->
            ###
            ###

            if @_channels.hasOwnProperty(channel)
                for subscriber in @_channels[channel]
                    subscriber(message)

     
     
        @$instance: () ->
            ###
            ###
     
            if not _class_instance_MessageQueue?
                # Create instance
                _class_instance_MessageQueue = new MessageQueue()
     
            return _class_instance_MessageQueue
     
        #####################
        ## PRIVATE METHODS ##
        #####################
 
    # Return the factory
    MessageQueue.$instance()
])