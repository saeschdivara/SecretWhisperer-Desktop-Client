module.exports = (grunt) ->

    scripting_path = 'js/'
    coffee_path = scripting_path + 'coffee/'

    coffee_apps_paths = [
        coffee_path + '*.coffee'
        coffee_path + '**/*.coffee'
    ]

    grunt.initConfig(
        pkg: grunt.file.readJSON('package.json'),

        coffee:

            coffee_app:
                options:
                    join: true,
                    sourceMap: true

                files:

                    ####################################################################
                    # BILLING CALCULATOR APP SECTION
                    ####################################################################
                    'js/dist/app.js': coffee_apps_paths



        watch:

            coffee_app_watch:
                files: [
                    coffee_apps_paths
                ]

                tasks: 'coffee:coffee_app'

    )

    grunt.loadNpmTasks('grunt-contrib-uglify')
    grunt.loadNpmTasks('grunt-contrib-coffee')
    grunt.loadNpmTasks('grunt-contrib-compass')
    grunt.loadNpmTasks('grunt-contrib-watch')

    ########################
    ## COFFEE WATCH TASKS ##
    ########################
    grunt.registerTask('watch-coffee-apps', ['watch:coffee_app_watch'])

    ######################
    ## SASS WATCH TASKS ##
    ######################

