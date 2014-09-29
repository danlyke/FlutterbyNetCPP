#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
// #include "cgicc/HTMLClasses.h"


#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
#include "stringutil.h"

#include <vector>
using namespace std;

const char *html_header =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "    <html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "\n"
    "    <head>\n"
    "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
    "    <title>Set Status</title>\n"
    "\n"
    "    <meta name=\"viewport\" content=\"width=device-width; initial-scale=1.0; maximum-scale=1.0;\">\n"
    "    <link rel=\"apple-touch-icon\" href=\"/iphone/setstatusicon.png\"/>\n"
    "\n"
    "    <style type=\"text/css\">\n"
    "    \@import url(\"/iphone/iphone.css\");\n"
    "</style>\n"
    "\n"
    "    <script type=\"text/javascript\" src=\"/iphone/orientation.js\"></script>\n"
    "    <script type=\"text/javascript\">\n"
    "    window.addEventListener(\"load\", function() { setTimeout(loaded, 100) }, false);\n"
    "\n"
    "function loaded() {\n"
    "    document.getElementById(\"page_wrapper\").style.visibility = \"visible\";\n"
    "    window.scrollTo(0, 1); // pan to the bottom, hides the location bar\n"
    "}\n"
    "</script>\n"
    "    <script>\n"
    "    function handler(location) {\n"
    "	var lat = document.getElementById(\"lat\");\n"
    "	var lon = document.getElementById(\"lon\");\n"
    "	lat.value = location.coords.latitude;\n"
    "	lon.value = location.coords.longitude;\n"
    "	var posaccuracy = document.getElementById(\"posaccuracy\");\n"
    "	posaccuracy.value = location.coords.accuracy;\n"
    "}\n"
    "navigator.geolocation.getCurrentPosition(handler);\n"
    "</script>\n"
    "    </head>\n"
    "\n"
    "    <body onorientationchange=\"updateOrientation();\">\n"
    "    <div id=\"page_wrapper\">\n"
    "    <h1>Set Status</h1>\n"
    "\n";


int main(int argc, char**argv, char **env)
{
    FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke'"));
    int limit = 50;
    cgicc::Cgicc cgi;


my $status = $cgi->param('status') if (defined($cgi->param('status')));

foreach ($cgi->param)
{
    print "$_ : ".$cgi->param($_)."<br>\n";
}

if (defined($status) && $status !~ /^\s*$/ && $cgi->param('pw') eq 'geflertz')
{
    my ($imagename, $thumbpath, $thumbwidth, $thumbheight);

    my $needsrebuild;
    if (defined($cgi->param('photofile'))
        && defined(my $ufh = $cgi->upload('photofile'))
        && defined($cgi->param('photoname'))
        && $cgi->param('photoname') ne '')
    {
        my $cwd = getcwd;
        $imagename = $cgi->param('photoname');
        my $writecmd = '/home/danlyke/bin/fby writeimagefile "'.$cgi->param('photoname').'"';
        print "Getting uploaded photo: $writecmd<br>\n";
        if (open my $outfh, '|-', $writecmd)
        {
            my $buffer;
            binmode $ufh;
            binmode $outfh;
            while (read($ufh, $buffer, 16384))
            {
                print $outfh $buffer;
            }
            close $outfh;
            close $ufh;
        }
        else
        {
            print "Unable to write image<br>\n";
        }
        chdir $cwd;
        $needsrebuild = 1;
    }

    
    print "<b>Posting status: $status</b><br>\n";
    use DBI;
    my $dbh = DBI->connect('DBI:Pg:dbname=flutterbynet;host=localhost',
			   'danlyke', 'danlyke')
	|| die "Unable to connect\n";
    
    my $lat = $cgi->param('lat');
    my $lon = $cgi->param('lon');
    $lat = 0 unless defined($lat) && $lat ne '';
    $lon = 0 unless defined($lat) && $lon ne '';
    my $posaccuracy = $cgi->param('posaccuracy');
    $posaccuracy = 0 if ($posaccuracy eq '');

    my @a=(0..9,'a'..'z','A'..'Z');
    my $xid = join('', map {$a[rand(@a)]} (1..8));

    my $sql = 'INSERT INTO statusupdate(status,locationset,latitude,longitude,posaccuracy,flutterby_update,twitter_update,facebook_update,identica_update,person_id,imagename, thumbnailpath, thumbnailwidth, thumbnailheight, xid) VALUES('
	.join(',',
	      $dbh->quote($status),
	      (defined($lat) && defined($lon) && $lat ne 0 && $lon ne 0) ? 'true' : 'false',
	      defined($lat) ? $dbh->quote($lat) : 0,
	      defined($lon) ? $dbh->quote($lon) : 0,
	      defined($posaccuracy) ? $dbh->quote($posaccuracy) : 0,
	      $cgi->param('flutterby') ? 'true' : 'false',
	      $cgi->param('twitter') ? 'true' : 'false',
	      $cgi->param('facebook') ? 'true' : 'false',
	      $cgi->param('identica') ? 'true' : 'false',
	      1,
	      $dbh->quote($imagename),
	      $dbh->quote($thumbpath),
	      $dbh->quote($thumbwidth),
	      $dbh->quote($thumbheight),
	      $dbh->quote($xid),
	)
	.')';
    $dbh->do($sql)
	|| print "<p><b>$dbh::errstr</b></p>\n";
    @a = $dbh->selectrow_array("SELECT CURRVAL(pg_get_serial_sequence('statusupdate', 'id'))");
    my $recid = $a[0];
    foreach ('flutterby', 'facebook', 'identica', 'twitter')
    {
	if (defined($cgi->param($_)))
	{
	    my $subsql = "INSERT INTO update_$_ (statusupdate_id) VALUES ($recid)";
	    $dbh->do($subsql)
		|| print "<p><b>$dbh::errstr</b>: $subsql</p>\n";
	}
    }

    print "<p><b>Success: $sql</b></p>\n";
    $cgi->param('status' => '');
    $dbh->disconnect();
    system('/home/danlyke/bin/fby scanimages >& /dev/null &')
        if $needsrebuild;
}

my $imagename;
{
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =
	localtime(time);
    $imagename = sprintf("%4.4d-%2.2d-%2.2d-%2.2d-%2.2d.jpg",
			 $year + 1900, $mon + 1, $mday, $hour, $min);
}

print <<EOF;
<div id="charcount">0</div>
    <form method="post" action="/cgi-bin/setstatus.pl" enctype="multipart/form-data">
EOF
    print $cgi->start_form;
print "Password: ". $cgi->textfield(-name => 'pw', -default => $cgi->param('pw'));
print <<EOF;
<br>
    <textarea id="status" name="status" rows="4" cols="40"></textarea>
    <br>
    <input type="checkbox" name="twitter" checked="1">Twitter
    <input type="checkbox" name="identica" checked="1">Identica
    <input type="checkbox" name="facebook" checked="1">Facebook
    <input type="checkbox" name="flutterby" >Flutterby
    <br>L/L: <input id="lat" name="lat" size="6" />
<input id="lon" name="lon" size="6"/>
    +-: <input id="posaccuracy" name="posaccuracy" size="4" />
<input type="button" name="Here" value="here" onClick="navigator.geolocation.getCurrentPosition(handler);" />
    <br>Photo: <input name="photofile" type="file" />
    <br>Photo Name: <input name="photoname" size="32" value="$imagename" />
EOF
    print "<br>".$cgi->submit('Save','save');
print $cgi->end_form;
print $cgi->hr;
print <<EOF;
<script type="text/javascript">

(function () {
    var div, txt;
    div = document.getElementById('charcount');
    txt = document.getElementById('status');
    txt.onkeyup = function () {
	div.innerHTML = txt.value.length;
    };
    txt.focus();
 })();
</script>
EOF

    print "\n</div>\n";
print $cgi->end_html();
print "\n";

