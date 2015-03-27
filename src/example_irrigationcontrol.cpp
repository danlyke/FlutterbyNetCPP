#include "fbynet.h"
#include "fbyregex.h"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

using namespace std;


const char htmlHeaderStuff[] =
    "<html>\n"
    "<head>\n"
    "<title>Irrigation Controller</title>\n"
    "</head>\n"
    "<body>\n"
    "<h1></h1>\n"
    "\n"
    "<table>\n"
    "<tr><th>Zone</th><th>Run Time</th><th>Mon</th><th>Tue</th><th>Wed</th><th>Thu</th><th>Fri</th><th>Sat</th><th>Sun</th></tr>\n";

const char htmlFooterStuff[] =
    "</table>\n"
    "\n"
    "</body>\n"
    "</html>\n";


FBYCLASSPTR(Valve);
FBYCLASS(Valve) : public FbyHelpers::BaseObj {
public:
    string name;
    int valve_num;
    int run_time;
    int remaining_run_time;
    int days;
    IntervalObjectPtr timer;
    

    Valve() : 
        BaseObj(BASEOBJINIT(Valve)),
        name(), valve_num(), run_time(), remaining_run_time(), days(),timer() {}
    void TurnOff()
    {
        cout << "Turning off valve " << valve_num << endl;
    }
    void TurnOn()
    {
        remaining_run_time = run_time * 60;
        cout << "Turning on valve " << valve_num << endl;
    }
};


void AddInput(stringstream &ss, int valve_id, string name, int size, string value, const char *type = "text")
{
    ss << "<input type=" << type << "\" size=\"" << size << "\" name=\""
       << name << "_" << valve_id << "\" value=\"" << value << "\" />";
}

const char *daysOfWeek[] =
{
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

string ValveHTML(ValvePtr valve)
{
    stringstream ss;
    ss << "<tr><th>" << (valve->valve_num + 1);
    AddInput(ss, valve->valve_num,
             "name", 16,
             valve->name);
    ss << "</th><td>";
    AddInput(ss, valve->valve_num,
             "time", 4,
             to_string(valve->run_time));
    
    int min = valve->remaining_run_time / 60;
    int secs = valve->remaining_run_time % 60;
    ss << " " << min << ":" << secs;
    ss << "</td>";
    for (int i = 0; i < 7; ++i)
    {
        ss << "<td><input type=\"checkbox\" name=\"valve_" << daysOfWeek[i]
           << "_" << valve->valve_num << "\" ";
        if (valve->days & (1 << i))
        {
            ss << " checked=\"1\" ";
        }
        ss << "/></td>";
    }
    ss << "<td><a href=\"/start/" << valve->valve_num << "\">Start</a></td>";
    ss << "</tr>\n\n";
    return ss.str();
}

int main(int argc, char **argv)
{
    Net *net = new Net();
    vector<ValvePtr> valves;
    for (int i = 0; i < 8; ++i)
    {
        ValvePtr valve(new Valve);
        valve->name = "Valve " + to_string(i + 1);
        valve->valve_num = i;
        valve->run_time = 15 + i;
        valve->days = 0xff;
        valves.push_back(valve);
    }

    Regex regexStart("^/start/(\\d+)");

    net->createServer([valves,&regexStart,net](SocketPtr socket)
                     {
                         HTTPRequestBuilderPtr requestBuilder
                             (new HTTPRequestBuilder
                              (socket,
                               [valves,&regexStart,net](HTTPRequestPtr request, HTTPResponsePtr response)
                               {
                                   RegexMatch match; 
                                   for (auto v = request->headers.begin();
                                        v != request->headers.end();
                                        ++v)
                                   {
                                       cout << "   " << v->first << " : " << v->second << endl;
                                   }
                                   if (request->path == "/")
                                   {
                                       response->writeHead(200);
                                       response->write(htmlHeaderStuff,sizeof(htmlHeaderStuff));
                                       for (auto valve : valves)
                                       {
                                           response->write(ValveHTML(valve));
                                       }
                                       
                                       response->end(htmlFooterStuff,sizeof(htmlFooterStuff));
                                   }
                                   else if (regexStart.Match(request->path, match))
                                   {
                                       cout << "Converting " << match.Match(1) << endl;
                                       size_t valve_num = stol(match.Match(1));
                                       if (valve_num < valves.size())
                                       {
                                           auto valve = valves[valve_num];
                                           valve->TurnOn();
                                           int valve_interval = 15;
                                           valve->timer = net->setInterval
                                               (
                                                   [valve, net, valve_interval]()
                                                   {
                                                       valve->remaining_run_time -= valve_interval;
                                                       if (valve->remaining_run_time <= 0)
                                                       {
                                                           valve->remaining_run_time = 0;
                                                           net->clearInterval(valve->timer);
                                                           valve->timer = IntervalObjectPtr();
                                                       }
                                                       valve->TurnOff();
                                                   },
                                                   valve_interval * 1000
                                                   );
                                       }
                                       response->redirect("/");
                                   }
                                   else if (!ServeFile("../../irrigation/html", request,response))
                                   {
                                       response->writeHead(404);
                                       response->end("<html><head><title>Nope!</title></head><body><h1>Ain't there, dude!</h1></body></html>\n");
                                   }
                               }
                                  ));
                         
                         socket->onData(
                             [socket, requestBuilder,net](const char *data, int length)
                             {
                                 requestBuilder->ReadData(data, length);
                             });
                     })->listen(5000);
    net->loop();
}
