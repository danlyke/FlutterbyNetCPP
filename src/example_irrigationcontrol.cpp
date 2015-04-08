#include "fbynet.h"
#include "fbyregex.h"
#include "fbystring.h"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "irrigation.h"

using namespace std;

string configFileName("irrigationcontrol.ini");

const char htmlHeaderStuff[] =
    "<html>\n"
    "<head>\n"
    "<title>Irrigation Controller</title>\n"
    "</head>\n"
    "<body>\n"
    "<h1></h1>\n"
    "\n"
    "<form method=\"POST\">\n";

const char htmlTableHeaderStuff[] =
    "<table>\n"
    "<tr><th rowspan=2 >Zone</th><th rowspan= 2 >Run Time</th>"
"<th colspan=2>Mon</th><th colspan=2>Tue</th><th colspan=2>Wed</th><th colspan=2>Thu</th><th colspan=2>Fri</th><th colspan=2>Sat</th><th colspan=2>Sun</th></tr>\n"
"<tr><th>AM</th><th>PM</th> <th>AM</th><th>PM</th> <th>AM</th><th>PM</th> <th>AM</th><th>PM</th> <th>AM</th><th>PM</th> <th>AM</th><th>PM</th> <th>AM</th><th>PM</th> </tr>\n";

const char htmlFooterStuff[] =
    "</table>\n"
    "<input type=\"submit\" name=\"Save\" value=\"Save\" />"
    "</form>\n"
    "\n"
    "</body>\n"
    "</html>\n";




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
    char achTime[16];
    snprintf(achTime,sizeof(achTime), "%d:%2.2d", min, secs);
    ss << " " << achTime;
    ss << "</td>";
    for (int i = 0; i < 7; ++i)
    {
        ss << "<td><input type=\"checkbox\" name=\"valve_" << daysOfWeek[i]
           << "_am_" << valve->valve_num << "\" ";
        if (valve->days & (1 << i))
        {
            ss << " checked=\"1\" ";
        }
        ss << "/></td>";
        ss << "<td><input type=\"checkbox\" name=\"valve_" << daysOfWeek[i]
           << "_pm_" << valve->valve_num << "\" ";
        if (valve->days & (1 << (i + 8)))
        {
            ss << " checked=\"1\" ";
        }
        ss << "/></td>";
    }
    ss << "<td><a href=\"/";
    ss << (valve->remaining_run_time ? "stop" : "start");
    ss << "/" << valve->valve_num << "\">";
    ss << (valve->remaining_run_time ? "stop" : "start");;
    ss << "</a></td>";
    ss << "</tr>\n\n";
    return ss.str();
}


void writeFiles(vector<ValvePtr> valves)
{
    string configFileNameBackup(configFileName + ".bak");
    rename(configFileName.c_str(), configFileNameBackup.c_str());
    ofstream configFile(configFileName);
    configFile << "[Main]" << endl;
    configFile << "start_time_1=" << startTime1 << endl;
    configFile << "start_time_2=" << startTime2 << endl;

    for (auto valve : valves)
    {
        configFile << "[Valve" << valve->valve_num << "]" << endl;
        configFile << "name=" << valve->name << endl;
        configFile << "valve_num=" << valve->valve_num << endl;
        configFile << "run_time=" << valve->run_time << endl;
        configFile << "days=" << valve->days << endl;
    }
}


Regex regexStart("^/start/(\\d+)");
Regex regexStop("^/stop/(\\d+)");


int main(int /* argc */, char ** /* argv */)
{
    Net *net = new Net();
    vector<ValvePtr> valves;

    using boost::property_tree::ptree;
    ptree pt;
    try {
        read_ini(configFileName, pt);
    } catch(std::exception const&  ex)
    {
    }

    for (int i = 0; i < 8; ++i)
    {
        ValvePtr valve(new Valve);
        valve->name = "Valve " + to_string(i);
        valve->valve_num = i;
        valve->run_time = 1;
        valve->days = -1;
        valves.push_back(valve);
    }

    for (auto &section :pt)
    {
        if (startswith(section.first, "Valve"))
        {
            string valveStr(section.first);
            int valveNum = valveStr[5] - '0';
            ValvePtr valve(valves[valveNum]);
            valve->valve_num = valveNum;

            for (auto& key : section.second)
            {
                if (key.first == "name")
                {
                    valve->name = key.second.get_value<std::string>();
                }
                if (key.first == "valve_num")
                {
                    valve->valve_num = key.second.get_value<int>();
                }
                if (key.first == "run_time")
                {
                    valve->run_time = key.second.get_value<int>();
                }
                if (key.first == "days")
                {
                    valve->days = key.second.get_value<int>();
                }
            }
        }
        if (startswith(section.first, "Main"))
        {
            for (auto& key : section.second)
            {
                if (key.first == "start_time_1")
                {
                    startTime1 = key.second.get_value<string>();
                }
                if (key.first == "start_time_2")
                {
                    startTime2 = key.second.get_value<string>();
                }
            }
        }
    }


    net->createServer([valves,net](SocketPtr socket)
                     {
                         HTTPRequestBuilderPtr requestBuilder
                             (new HTTPRequestBuilder
                              (socket,
                               [valves,net](HTTPRequestPtr request, HTTPResponsePtr response)
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
                                       if (request->method == "POST")
                                       {
                                           BodyParserURLEncodedPtr parseptr(new BodyParserURLEncoded);
                                           NameValuePairPtr nvpptr(new NameValuePair);
                                           parseptr->onNameValue([nvpptr](string name, string value)
                                                                 {
                                                                     nvpptr->nvp[name] = value;
                                                                 });

                                           request->onData([parseptr](const char *data, size_t length)
                                                            {
                                                                cout << "Got data " << length << endl;
                                                                parseptr->on_data(data,length);
                                                            });

                                           request->onEnd([parseptr, nvpptr, valves, response]()
                                                           {
                                                               cout << "Processing request onEnd" << endl;
                                                               for (auto valve : valves)
                                                               {
                                                                   char valveIdch[2];
                                                                   valveIdch[0] = '0' + valve->valve_num;
                                                                   valveIdch[1] = '\0';
                                                                   string valveId(valveIdch);

                                                                   auto value = nvpptr->nvp.find("name_" + valveId);
                                                                   if (value != nvpptr->nvp.end())
                                                                   { valve->name  = value->second; }

                                                                   value = nvpptr->nvp.find("time_" + valveId);
                                                                   if (value != nvpptr->nvp.end())
                                                                   { valve->run_time  = stoi(value->second); }

                                                                   for (int dow = 0; dow < 7; ++dow)
                                                                   {
                                                                       string strValve("valve_");

                                                                       value = nvpptr->nvp.find(
                                                                           strValve + daysOfWeek[dow] + "_am_" + valveId);
                                                                       if (value != nvpptr->nvp.end()
                                                                           && value->second == "on")
                                                                       {
                                                                           valve->days |= (1 << dow);
                                                                       }
                                                                       else
                                                                       {
                                                                           valve->days &= ~(1 << dow);
                                                                       }
                                                                           
                                                                       value = nvpptr->nvp.find(
                                                                           strValve + daysOfWeek[dow] + "_pm_" + valveId);
                                                                       if (value != nvpptr->nvp.end()
                                                                           && value->second == "on")
                                                                       {
                                                                           valve->days |= (1 << (dow + 8));
                                                                       }
                                                                       else
                                                                       {
                                                                           valve->days &= ~(1 << (dow + 8));
                                                                       } 
                                                                   }
                                                               }
                                                               auto value = nvpptr->nvp.find("start_time_1");
                                                               if (value != nvpptr->nvp.end())
                                                               { startTime1 = value->second; }

                                                               value = nvpptr->nvp.find("start_time_2");
                                                               if (value != nvpptr->nvp.end())
                                                               { startTime2 = value->second; }

                                                               response->redirect("/"); 
                                                               response->end("");
                                                               writeFiles(valves);
                                                           });
                                           
                                       }
                                       else
                                       {
                                           response->writeHead(200);
                                           response->write(htmlHeaderStuff,sizeof(htmlHeaderStuff));
                                           stringstream ss;
                                           ss << "Start time: <input type=\"time\" size=\"8\" name=\"start_time_1\" value=\"";
                                           ss << startTime1;
                                           ss << "\" />\n";
                                           ss << "Start time: <input type=\"time\" size=\"8\" name=\"start_time_2\" value=\"";
                                           ss << startTime2;
                                           ss<< "\" />\n";
                                           response->write(ss.str());

                                           response->write(htmlTableHeaderStuff, sizeof(htmlTableHeaderStuff));
                                           for (auto valve : valves)
                                           {
                                               response->write(ValveHTML(valve));
                                           }
                                       
                                           response->end(htmlFooterStuff,sizeof(htmlFooterStuff));
                                       }
                                   }
                                   else if (regexStart.Match(request->path, match))
                                   {
                                       cout << "Converting " << match.Match(1) << endl;
                                       size_t valve_num = stol(match.Match(1));
                                       if (valve_num < valves.size())
                                       {
                                           auto valve = valves[valve_num];
                                           valve->TurnOn();
                                           int valve_interval = 1;
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
                                                           valve->TurnOff();
                                                       }
                                                   },
                                                   valve_interval * 1000
                                                   );
                                       }
                                       response->redirect("/");
                                   }
                                   else if (regexStop.Match(request->path, match))
                                   {
                                       cout << "Converting " << match.Match(1) << endl;
                                       size_t valve_num = stol(match.Match(1));
                                       if (valve_num < valves.size())
                                       {
                                           auto valve = valves[valve_num];
                                           valve->TurnOff();
                                           net->clearInterval(valve->timer);
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
