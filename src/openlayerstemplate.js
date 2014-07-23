// FbySection: preamble




var map$mapnum;

function makeMap$mapnum() 
{
	var newlayer;

	var epsg4326proj = new OpenLayers.Projection("EPSG:4326")
	var map = new OpenLayers.Map('map$mapnum');
	var layerSwitcher = new OpenLayers.Control.LayerSwitcher();
	map.addControl(layerSwitcher);
	var selectControl;


	var osm = new OpenLayers.Layer.OSM.MapnikLocalProxy("OpenStreetMap");
	osm.setIsBaseLayer(true);

    // the SATELLITE layer has all 22 zoom level, so we add it first to
    // become the internal base layer that determines the zoom levels of the
    // map.
    var gsat = new OpenLayers.Layer.Google(
        "Google Satellite",
        {type: google.maps.MapTypeId.SATELLITE, numZoomLevels: 22, visibility: false,
		isBaseLayer: true,
		}
    );
	gsat.setIsBaseLayer(true);

    var gphy = new OpenLayers.Layer.Google(
        "Google Physical",
        {type: google.maps.MapTypeId.TERRAIN, visibility: false,
		isBaseLayer: true,
		}
    );
	gphy.setIsBaseLayer(true);

    var gmap = new OpenLayers.Layer.Google(
        "Google Streets", // the default
        {numZoomLevels: 20, visibility: false,
		isBaseLayer: true,
		}
    );
	gmap.setIsBaseLayer(true);

    var ghyb = new OpenLayers.Layer.Google(
        "Google Hybrid",
        {type: google.maps.MapTypeId.HYBRID, numZoomLevels: 22, visibility: false,
		isBaseLayer: true,
		}
    );
	ghyb.setIsBaseLayer(true);


    map.addLayers([osm, gsat, gphy, gmap, ghyb]);

    // Google.v3 uses EPSG:900913 as projection, so we have to
    // transform our coordinates
    map.setCenter(new OpenLayers.LonLat($lon, $lat).transform(
					  epsg4326proj,
					  map.getProjectionObject()
				  ), $zoom);

	// FbySection: KML
	newlayer = new OpenLayers.Layer.Vector
	("KML",
	 {
		 strategies: [new OpenLayers.Strategy.Fixed()],
		 protocol: new OpenLayers.Protocol.HTTP
		 ({
			  url: "$kmlurl",
			  format: new OpenLayers.Format.KML({
													extractStyles: true, 
													extractAttributes: true,
													maxDepth: 2
												})
          })
     });
	selectControl = new OpenLayers.Control.SelectFeature(
		newlayer,
		{onSelect: function (feature) 
		 {
			 selectedFeature = feature;
			 popup = new OpenLayers.Popup.FramedCloud(
				 "chicken", 
				 feature.geometry.getBounds().getCenterLonLat(),
				 new OpenLayers.Size(100,150),
				 "<div style='font-size:.8em'><b>Name:</b>"+feature.attributes.name+"<br><b>Description:</b>"+feature.attributes.description+"</div>",
				 null, true,
				 function () {selectControl.unselect(selectedFeature);}
				 );
			 feature.popup = popup;
			 map.addPopup(popup);
		 },
		 onUnselect: function (feature) 
		 {
			 map.removePopup(feature.popup);
			 feature.popup.destroy();
			 feature.popup = null;
		 },
		});
	map.addControl(selectControl);
	selectControl.activate();
	map.addLayer(newlayer);

	// FbySection: GeoRSS
	newlayer = new OpenLayers.Layer.GeoRSS( '$georsslayer', '$georssurl');
	selectControl = new OpenLayers.Control.SelectFeature(
		newlayer,
		{onSelect: function (feature) 
		 {
			 selectedFeature = feature;
			 popup = new OpenLayers.Popup.FramedCloud(
				 "chicken", 
				 feature.geometry.getBounds().getCenterLonLat(),
				 new OpenLayers.Size(100,150),
				 "<div style='font-size:.8em'><b>Name:</b>"+feature.attributes.name+"<br><b>Description:</b>"+feature.attributes.description+"</div>",
				 null, true,
				 function () {selectControl.unselect(selectedFeature);}
			 );
			 feature.popup = popup;
			 map.addPopup(popup);
		 },
		 onUnselect: function (feature) 
		 {
			 map.removePopup(feature.popup);
			 feature.popup.destroy();
			 feature.popup = null;
		 },
		});
	map.addControl(selectControl);
	selectControl.activate();
	map.addLayer(newlayer);

	// FbySection: markerlayerbegin
	var markers = new OpenLayers.Layer.Markers("Markers");
	map.addLayer(markers);
	var marker;

	// FbySection: marker
	{
		var feature = new OpenLayers.Feature(markers, new OpenLayers.LonLat($lon,$lat).transform(
												 epsg4326proj,
												 map.getProjectionObject()));
		feature.closeBox = true;
		feature.popupClass =  OpenLayers.Class(OpenLayers.Popup.FramedCloud, {
												   'autoSize': true,
												   'maxSize': new OpenLayers.Size(300,200)
											   });
		feature.data.popupContentHTML = "$caption";
        feature.data.overflow = "auto";
        
		marker = feature.createMarker();
		
		var markerClick = function (evt) {
			if (this.popup == null) {
				this.popup = this.createPopup(this.closeBox);
				map.addPopup(this.popup);
				this.popup.show();
			} else {
				this.popup.toggle();
			}
			currentPopup = this.popup;
			OpenLayers.Event.stop(evt);
        };
		marker.events.register("mousedown", feature, markerClick);
		markers.addMarker(marker);
	} 


	// FbySection: vectorlayerpreamble
	{
		var layerGeometry = new OpenLayers.Layer.Vector("Lines",{projection: map.getProjectionObject()});
		map.addLayer(layerGeometry);
		var points = [];

		// FbySection: vectorlayerpoint
		{
			var point = new OpenLayers.Geometry.Point($lon,$lat);
			points.push(point.transform(epsg4326proj, map.getProjectionObject()));
		}
		// FbySection: vectorlayerpost
		var geometry = new OpenLayers.Geometry.LineString(points);
		var feature = new OpenLayers.Feature.Vector(geometry, null, {
														strokeColor: "#$color",
														strokeOpacity: 0.7,
														strokeWidth: 5
													});
		layerGeometry.addFeatures(feature);
	}

    // FbySection: gpstracklayer
    {
        var urlbase = '/cgi-bin/mapserver.pl?type=';
        var url = '';
        var fromdate = '$fromdate';
        var todate = '$todate';
        var imgoffset = '$imgoffset';

        if (fromdate != "")
        {
            url = url + "&from=" + fromdate;
        }

        if (todate != "")
        {
            url = url + "&to=" + todate;
        }

        if (imgoffset != "")
        {
            url = url + "&imgoffset=" + imgoffset;
        }
        var linelayername = "Line Layer - $fromdate - $todate";
        var markerlayername = "Marker Layer - $fromdate - $todate"
        var imagelayername = "Image Layer - $fromdate - $todate"
        loadLineLayerIntoMap(map, linelayername, new Array());
        loadMarkersIntoMap(map, markerlayername, "blue", new Array());
        loadMarkersIntoMap(map, imagelayername, "camera", new Array());
        $.ajax({
            url: urlbase+'line'+url,
            dataType: 'json',
            success: function (data)
            {
                var pll = loadLineLayerIntoMap(map, linelayername, data);
                map.setCenter(pll, $zoomlevel);
            }
        });

        $.ajax({
            url: urlbase+'marks'+url,
            dataType: 'json',
            success: function (data)
            {
                var pll = loadMarkersIntoMap(map, markerlayername, "blue", data);
                map.setCenter(pll, $zoomlevel);
            }
        });

        $.ajax({
            url: urlbase+'image'+url,
            dataType: 'json',
            success: function (data)
            {
                var pll = loadMarkersIntoMap(map, imagelayername, "camera", data);
                map.setCenter(pll, $zoomlevel);
            }
        });
    }

	// FbySection: post
	map$mapnum = map;
}
addLoadEvent(makeMap$mapnum);

