

log = (message) ->
    jQuery('#message-board').append("<span>#{message}</span>")


getRandomHexString = () ->
    ###
     Generates random hex number string

     @return {String}
    ###
    return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1)