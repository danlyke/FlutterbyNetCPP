SELECT AddGeometryColumn('zip_codes', 'position', 4326, 2);
UPDATE zip_codes SET position=ST_SetSRID(ST_MakePoint(latitude, longitude), 4326);
