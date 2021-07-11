v = 1000

[Lat,lon,alt]=Local2Spherical([0,v,0], [30, 50, 0])
[Lat1,lon1,alt1]=Local2Spherical([v,0,0], [Lat,lon,alt])

[Lat,lon,alt]=Local2Spherical([v,0,0], [30, 50, 0])
[Lat2,lon2,alt2]=Local2Spherical([0,v,0], [Lat,lon,alt])

[dx,dy,dz]=Spherical2Local([Lat2,lon2,alt2], [Lat1,lon1,alt1])

error_percent = [dx/v, dy/v, dz/v]*100