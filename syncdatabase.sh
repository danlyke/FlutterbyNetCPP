mkdir -p /home/danlyke/websites/flutterby.net/public_html_new/wiki/
rsync -avz danlyke@www.flutterby.net:websites/flutterby.net/public_html_static/files ~/websites/flutterby.net/public_html_new/
rsync -avz danlyke@www.flutterby.net:/home/danlyke/websites/flutterby.net/mediawiki/images /home/danlyke/websites/flutterby.net/public_html_new/wiki/
dropdb flutterbynet
createdb flutterbynet
ssh www.flutterby.net 'pg_dump flutterbynet' | psql flutterbynet
