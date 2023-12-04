#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define PI acos(-1) // Pi 3.1415...
#define EARTHRAD 6371 // km earth radius

struct position {  float lat, lon; };

float deg2rad(float d){  return d * PI/180.0; }

float distance(struct position posA, struct position posB){
    float s = sin(deg2rad(posA.lat))*sin(deg2rad(posB.lat));
    float c = cos(deg2rad(posA.lat))*cos(deg2rad(posB.lat))*cos(deg2rad(posB.lon-posA.lon));
    return EARTHRAD * acos(s + c);
}

int bad_position(struct position pos){
    int a,b;
    if (pos.lat < -90 || pos.lat > 90) a = 1; else a = 0;
    if (pos.lon < -180 || pos.lon > 180) b = 1; else b = 0;
    if (a||b) printf("Value out of range! ( -90<lat<90 ; -180<lon<180 )\n");
    return (a || b); // if all within limits returns 0
}

int bad_speed(float v){
    int a = (v<=0 || v>100) ; // 0/1
    if (a) printf("Value out of range! ( 0<v<100 )");
    return a;
}



float get_position(char coord[]){
    float pos=0.0, deg=0.0, min=0.0, low, high;
    int test1=0, test2=0; // store bool 1/0 to test for good values
    if (!strcmp(coord,"LAT")){ low=-90.0; high=90.0;} else {low=-180.0; high=180.0;}

    // get coordinate
    do{
        do {
        printf("Enter %s [dd.ddd 0 or dd mm.mmm]:",coord);
        scanf("%f %f", &deg, &min);
        test1 = (deg>=low) && (deg<=high) && (min>=0.0) && (min<=60.0); // test lat values are in range 1 good, 0 bad
        if (!test1) printf("Value out of range! (%f<%s<%f)\n", low, coord, high);
        } while (!test1); // repeat if out of range values
        if (deg>0) pos = deg + min/60.0; else pos = deg - min/60.0;
        test2 = (pos<high) && (pos>low); // check that total sum is not out of range 1 good, 0 bad
        if (!test2) printf("%s out of range!\n", coord);
    } while(!test2); 

    return pos;
}

int main(){
    // VAR INITIALIZATIONS
    struct position posA, posB; // position A and B .lat .lon
    float dist, speed, xbtDist; //distance A-B, ship's speed in knots, distance between deployments km
    float travelTimeh, travelTimed; // travel time hours, days
    float nextLaunch, totalMinutes; // time till next launch, minutes before empty autolauncher
    time_t nowUnix, etaUnix, reloadUnix; // long long to store unix time -> seconds since 1/1/1970, now, eta, reload by
    struct tm *utcTime, *etaTime, *reloadTime; // pointer to store time as tm struct: utc now, utc eta, utc reload by
    int xbtLaunches, xbtLoaded; // number of xbts to deploy, xbts loaded in autoaluncher

    // USER INPUT
    printf("                            *** XBT CRUISE ASSISTANT ***                \n");
    printf("************************************************************************************\n");
    printf("* Calculate Distance A-B |travel time |ETA |time to reload AL |XBTs to launch |    *\n");
    printf("* Points of interest: 30.33 -81.63 (JAX), 25.77 -80.16 (MIA), 18.45 -66.10 (SJU)   *\n");
    printf("* Exit with CTRL+C                                                                 *\n");
    printf("************************************************************************************\n\n");
    
    printf("---------------------------------Distance A-B---------------------------------------\n");
    printf("POSITION A:\n");
    posA.lat = get_position( "LAT");
    posA.lon = get_position("LON");
    printf("POSITION B:\n");
    posB.lat = get_position("LAT");
    posB.lon = get_position("LON");

    do {
        printf("Enter speed [kt]:");
        scanf("%f",&speed);
    } while (speed<=0 || speed>100);
    
    // RESULTS
    // distance
    dist = distance(posA, posB); // calculate distance between pos A and pos B
    // travel time
    travelTimeh = dist/(speed*1.852); // time in h
    travelTimed = travelTimeh/24.0; // time in days
    // utc time
    time(&nowUnix); // store unix time in unixTime variable - pass by reference
    utcTime = gmtime(&nowUnix); // convert unix time to tm struct format then access as utcTime.tm_hour etc.

    // OUTPUT
    printf("-->\n");
    printf("Current time:       %02d/%02d/%4d %02d:%02d:%02d UTC\n", 
        utcTime->tm_mon + 1, utcTime->tm_mday, utcTime->tm_year + 1900,
        utcTime->tm_hour,utcTime->tm_min, utcTime->tm_sec);
    printf("Positions:          A: %.3f %.3f | B: %.3f %.3f\n", posA.lat, posA.lon, posB.lat, posB.lon);

    if(dist>1.0){
        printf("Distance:           %.1f km\n",dist); }
    else { 
        printf("Distance:           %.1f m\n",dist*1000.0); }

    printf("Travel time:        %.1f hours (%.1f days)\n",travelTimeh, travelTimed);

    // eta
    etaUnix = nowUnix + travelTimeh * 3600; // estimated time of arrival utc unix
    etaTime = gmtime(&etaUnix);
    printf("Estimated arrival:  %02d/%02d/%4d %02d:%02d:%02d UTC\n\n", 
        etaTime->tm_mon + 1, etaTime->tm_mday, etaTime->tm_year + 1900,
        etaTime->tm_hour,etaTime->tm_min, etaTime->tm_sec);
    // printf("########################################################\n\n");
    // printf("--------------------------------------------------------\n\n");

    printf("---------------------------------XBT deployments------------------------------------\n");
    printf("Distance between deployments [km]:");
    scanf("%f", &xbtDist);
    printf("-->\n");
    xbtLaunches = (int)(dist/xbtDist);
    if (xbtDist > 0)  
        printf("XBTs to deploy:     %d (%.1f box)\n\n", xbtLaunches, xbtLaunches/12.0);
    // printf("########################################################\n\n");
    // printf("--------------------------------------------------------\n\n");
    
    printf("---------------------------------Empty Autolauncher---------------------------------\n");
    printf("XBTs loaded:");
    scanf("%d", &xbtLoaded);
    printf("Time to next launch [minutes]:");
    scanf("%f", &nextLaunch);
    printf("-->\n");
    totalMinutes = (xbtDist/(speed*1.852)) * xbtLoaded * 60 + nextLaunch; // minutes till AL is empty
    
    time(&nowUnix); // update unix time
    reloadUnix = nowUnix + totalMinutes * 60 ; // unix time empty autolauncher in seconds
    reloadTime = gmtime(&reloadUnix); // convert to tm struct format
    printf("Empty launch in:    %.1f hours (%.1f minutes)\n", totalMinutes/60.0, totalMinutes);
    printf("Reload by:          %02d/%02d/%4d %02d:%02d:%02d UTC\n", 
        etaTime->tm_mon + 1, reloadTime->tm_mday, reloadTime->tm_year + 1900,
        reloadTime->tm_hour,reloadTime->tm_min, reloadTime->tm_sec);
    // printf("--------------------------------------------------------\n\n");
    // printf("########################################################\n\n");
    printf("-----------------------------------------End----------------------------------------\n");
    

    return 0;
}