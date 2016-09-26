chatApp = angular.module('chatApp', [])
chatApp.config(['$compileProvider', ($compileProvider) ->
	$compileProvider.aHrefSanitizationWhitelist(/^\s*(stream):/)
	$compileProvider.imgSrcSanitizationWhitelist(/^\s*(stream):/)
])