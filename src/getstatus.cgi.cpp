#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
using namespace std;

int main(int argc, char**argv, char **env)
{
    FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke'"));


    vector<StatusUpdatePtr> statuses;

    string sql("...");
    db->Load(statuses, sql.c_str());
    return 0;
}


//
//#!/usr/bin/perl -w                                                                                    
//use strict;
//use CGI;
//
//my $cgi = CGI->new();
//
//use DBI;
//my $dbh = DBI->connect('DBI:Pg:dbname=flutterbynet;host=localhost',
//                           'danlyke', 'danlyke')
//    || die "Unable to connect\n";
//
//my $sql = 'SELECT statusupdate.*, person.name AS name FROM statusupdate, person WHERE statusupdate.person_id=person.id';
//my $limit = 50;
//
//if ($cgi->param('id'))
//{
//$sql .= ' AND xid='.$dbh->quote($cgi->param('id'));
//}
//elsif ($cgi->param('q'))
//{
//my $q = $cgi->param('q');
//my @a = split /\W+/, $q;
//    $sql .= " AND "
//        .join(' AND ',
//                  map { "to_tsvector('english',status) @@ to_tsquery('english',"
//                  .$dbh->quote($_).')' } @a);
//$limit = 50;
//}
//
//$sql .= " ORDER BY id DESC LIMIT $limit";
//
//print $cgi->header;
//print <<EOF;
//<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">                                                                                  
//<html xmlns="http://www.w3.org/1999/xhtml">                                                           
//<head>                                                                                                
//    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />                             
//        <title>Status</title>                                                                             
//        <style type="text/css">\@import "/screen.css";                                                        
//dd { margin-top: .5em; margin-bottom: 1em; }                                                          
//dt { border: solid  1px #000;                                                                         
//background-color: #fdf; }                                                                         
//</style>                                                                                              
//                                                                                                      
//<link rel="icon" href="/favicon.ico" type="image/ico"></link>                                         
//</head>                                                                                               
//                                                                                                      
//<body>                                                                                                
//                                                                                                      
//<form>                                                                                                
//<input name="q" type="text"><input type="submit" name="go" value="search" />                          
//</form>                        #!/usr/bin/perl -w                                                                                    
//use strict;
//use CGI;
//
//my $cgi = CGI->new();
//
//use DBI;
//my $dbh = DBI->connect('DBI:Pg:dbname=flutterbynet;host=localhost',
//                           'danlyke', 'danlyke')
//    || die "Unable to connect\n";
//
//my $sql = 'SELECT statusupdate.*, person.name AS name FROM statusupdate, person WHERE statusupdate.person_id=person.id';
//my $limit = 50;
//
//if ($cgi->param('id'))
//{
//$sql .= ' AND xid='.$dbh->quote($cgi->param('id'));
//}
//elsif ($cgi->param('q'))
//{
//my $q = $cgi->param('q');
//my @a = split /\W+/, $q;
//    $sql .= " AND "
//        .join(' AND ',
//                  map { "to_tsvector('english',status) @@ to_tsquery('english',"
//                  .$dbh->quote($_).')' } @a);
//$limit = 50;
//}
//
//$sql .= " ORDER BY id DESC LIMIT $limit";
//
//print $cgi->header;
//print <<EOF;
//<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">                                                                                  
//<html xmlns="http://www.w3.org/1999/xhtml">                                                           
//<head>                                                                                                
//    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />                             
//        <title>Status</title>                                                                             
//        <style type="text/css">\@import "/screen.css";                                                        
//dd { margin-top: .5em; margin-bottom: 1em; }                                                          
//dt { border: solid  1px #000;                                                                         
//background-color: #fdf; }                                                                         
//</style>                                                                                              
//                                                                                                      
//<link rel="icon" href="/favicon.ico" type="image/ico"></link>                                         
//</head>                                                                                               
//                                                                                                      
//<body>                                                                                                
//                                                                                                      
//<form>                                                                                                
//<input name="q" type="text"><input type="submit" name="go" value="search" />                          
//</form>                       
//<pre>$sql</pre>                                                                                       
//<div class="content">                                                                                 
//    <dl>                                                                                              
//EOF                                                                                                   
//
//my $sth = $dbh->prepare($sql)
//    || die $dbh->errstr;
//$sth->execute()
//    || die $sth->errstr;
//
//while (my $row = $sth->fetchrow_hashref)
//{
//    my $entry = $row->{status};
//    $entry =~ s/\&/\&amp\;/g;
//    $entry =~ s/\</\&lt\;/g;
//    $entry =~ s/\>/\&gt\;/g;
//    $entry =~ s/\'/\&apos\;/g;
//    $entry =~ s/\"/\&quot\;/g;
//    $entry =~ s/\r\n(\r\n)+/\<p\/\>/g;
//    $entry =~ s/\r\r+/\<p\/\>/g;
//    $entry =~ s/\n\n+/\<p\/\>/g;
//    print "<dt style=\"clear: left\"><strong>$row->{name}</strong> - <a href=\"./getstatus.pl?id=$row    ->{xid}\">$row->{entered}</a> &mdash; ";
//    print '<small>';
//    for (qw/twitter facebook flutterby identica/)
//    {
//        print "$_ ( $row->{$_.'_update'} / $row->{$_.'_updated'} ) ";
//    }
//    print "TwitterID: $row->{twitterid}" if ($row->{twitterid});
//    print "</small></td><dd>";
//
//    my $outputimage;
//    if ($row->{imagename})
//    {
//        my $sql = 'SELECT * FROM imageinstance WHERE imagename='.$dbh->quote($row->{imagename}).' ORDER BY width LIMIT 1';
//        my $stinst = $dbh->prepare($sql);
//
//        $stinst->execute();
//        if (my $thumbnail = $stinst->fetchrow_hashref)
//        {
//            my $thumb = $thumbnail->{filename};
//            $thumb =~ s/^.*public_html.*?\///;
//            print "<a href=\"http://www.flutterby.net/Image:$row->{imagename}\"><img src=\"http://www.flutterby.net/$thumb\" "
//                ."width=$thumbnail->{width} height=$thumbnail->{height} align=left></a>\n";
//            $outputimage = 1;
//        }
//    }
//    print "$entry";
//    print '<br clear="left" />' if ($outputimage);
//    print "</dd>\n";
//}
//print <<EOF;
//</dl>                                                                                                 
//</div>                                                                                                
//</body>                                                                                               
//</html>                                                                                               
//EOF                                                                                                   
//
//$dbh->disconnect();

