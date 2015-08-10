module.exports = (grunt) ->

    scripting_path = 'js/'
    coffee_path = scripting_path + 'coffee/'

    coffee_apps_paths = [
        coffee_path + '*.coffee'
        coffee_path + '**/*.coffee'
    ]

    style_path = 'css/'
    sass_path = style_path + 'sass/'

    sass_style_path = [
        sass_path + '*.sass'
        sass_path + '**/*.sass'
    ]

    grunt.initConfig(
        pkg: grunt.file.readJSON('package.json'),

        coffee:

            coffee_app:
                options:
                    join: true,
                    sourceMap: true

                files:
                    'js/dist/app.js': coffee_apps_paths


        compass:
            app:
                outputStyle: 'nested',
                debugsass: true,
                options:
                    config: style_path + 'config.rb',
                    sassDir: style_path + 'sass',
                    cssDir: style_path + 'stylesheets',
                    environment: 'development'



        watch:

            coffee_app_watch:
                files: [
                    coffee_apps_paths
                ]

                tasks: 'coffee:coffee_app'


            app_style_watch:
                files: [
                    sass_style_path
                ]

                tasks: 'compass:app'

    )

    grunt.loadNpmTasks('grunt-contrib-uglify')
    grunt.loadNpmTasks('grunt-contrib-coffee')
    grunt.loadNpmTasks('grunt-contrib-compass')
    grunt.loadNpmTasks('grunt-contrib-watch')

    ########################
    ## COFFEE WATCH TASKS ##
    ########################
    grunt.registerTask('watch-coffee-apps', ['watch:coffee_app_watch'])
    grunt.registerTask('watch-app-styling', ['watch:app_style_watch'])

    ######################
    ## SASS WATCH TASKS ##
    ######################

