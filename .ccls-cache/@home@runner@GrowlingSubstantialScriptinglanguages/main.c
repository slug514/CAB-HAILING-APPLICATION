#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <termios.h>


#define MAX_LOCATIONS 25
#define MAX_DRIVERS 80
#define MAX_DRIVERS_TYPE 16
#define MAX_VEHICLE_TYPE_LEN 50
#define MAX_CAR_TYPE_LEN 20
#define MAX_DRIVER_NAME_LEN 50
#define MAX_PHONE_NUMBER_LEN 15
#define MAX_VEHICLE_NUMBER_LEN 15
#define MAX_FILENAME_LEN 100

int n_mini, n_sedan, n_suv, n_auto, n_bike;

char username_check[20], password_check[20];

typedef struct {
  char username[20];
  char password[20];
  char first_name[50];
  char last_name[50];
  char phoneNumber[20];
  char email[20];
} User;

User user;

typedef struct {
  int id;
  char name[50];
  float latitude;
  float longitude;
} Location;

typedef struct {
  int id;
  char name[50];
  float latitude;
  float longitude;
  char phoneNumber[15];
  char carNumber[15];
  char carType[15];
  int pickupLocation;
  int dropLocation;
  int availability;
  double etaToPickup;
  float rating;
} Driver;

typedef struct {
  int numLocations;
  int numDrivers;
  int numMini;
  int numSedan;
  int numSuv;
  int numBike;
  int numAuto;
  Location locations[MAX_LOCATIONS];
  Driver drivers[MAX_DRIVERS];
  Driver Mini[MAX_DRIVERS_TYPE];
  Driver Sedan[MAX_DRIVERS_TYPE];
  Driver SUV[MAX_DRIVERS_TYPE];
  Driver Bike[MAX_DRIVERS_TYPE];
  Driver Auto[MAX_DRIVERS_TYPE];
  double distances[MAX_LOCATIONS][MAX_LOCATIONS];
} Graph;

typedef struct {
  char name[MAX_VEHICLE_TYPE_LEN];
  float fareRate;
  int availability;
  float fareperkm;
} CarType;

typedef struct {
  char name[MAX_DRIVER_NAME_LEN];
  char phoneNumber[MAX_PHONE_NUMBER_LEN];
  char carPlateNumber[MAX_VEHICLE_NUMBER_LEN];
  int cabCapacity;
  char CarType[10];
} driver;

typedef struct {
  char CarType[MAX_CAR_TYPE_LEN];
  char requestedTime[10];
} BookingRequest;

void print_availcabs(CarType CarTypes[], float distance) {
  printf("          \033[1;40m\033[1;37m Available Cab Types: \033[0m\n");
  for (int i = 0; i < 5; i++) {
    if (CarTypes[i].availability == 1) {
      printf("%d. %s - Fare Rate: %.2f\n", i, CarTypes[i].name,
             CarTypes[i].fareRate + ((distance - 3) * CarTypes[i].fareperkm));
    }
  }
}

void readFeedbackFile() {
    FILE *file = fopen("feedback.txt", "r");
    if (file != NULL) {
        printf("\n\033[1;40m\033[1;37m Feedback Ratings: \033[0m\n");
        char line[100];
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }
        fclose(file);
    } else {
        printf("Error opening feedback file.\n");
    }
}

float printEstimate(CarType CarTypes[], float distance, int i) {
  int k;
  k = CarTypes[i].fareRate + ((distance - 3) * CarTypes[i].fareperkm);

  return k;
}

void loadingAnimation(int percentage) {
  int e; // Changed variable name from i to e
  printf("Finding a Nearest Cab    ");
  for (e = 0; e <= percentage / 10; e++) {
    printf("|");
  }
  for (; e <= 10; e++) {
    printf(".");
  }
  printf(" %d%%\r", percentage);
  fflush(stdout);
}

void assignCarTypes(Driver drivers[], Graph *graph, int numDrivers, int numMini,int numSedan, int numSuv, int numBike, int numAuto) {
  int i, miniIndex = 0, sedanIndex = 0, suvIndex = 0, bikeIndex = 0,
         autoIndex = 0;
  for (i = 0; i < numDrivers; i++) {
    if (i < numMini) {
      graph->Mini[miniIndex++] = drivers[i];
      strcpy(drivers[i].carType, "mini \U0001F695");
    } else if (i < numMini + numSedan) {
      graph->Sedan[sedanIndex++] = drivers[i];
      strcpy(drivers[i].carType, "sedan \U0001F697");
    } else if (i < numMini + numSedan + numSuv) {
      graph->SUV[suvIndex++] = drivers[i];
      strcpy(drivers[i].carType, "suv \U0001F699");
    } else if (i < numMini + numSedan + numSuv + numBike) {
      graph->Bike[bikeIndex++] = drivers[i];
      strcpy(drivers[i].carType, "bike");
    } else if (i < numMini + numSedan + numSuv + numBike + numAuto) {
      graph->Auto[autoIndex++] = drivers[i];
      strcpy(drivers[i].carType, "auto \U0001F6FA");
    } else {
      break;
    }
  }

  graph->numMini = miniIndex;
  graph->numSedan = sedanIndex;
  graph->numSuv = suvIndex;
  graph->numBike = bikeIndex;
  graph->numAuto = autoIndex;
}

double Haversine(double lat1, double lon1, double lat2, double lon2) {
  double dLat = (lat2 - lat1) * M_PI /
                180.0; // distance between lats & convert to radians
  double dLon = (lon2 - lon1) * M_PI / 180.0;

  double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1 * M_PI / 180.0) *
                                                 cos(lat2 * M_PI / 180.0) *
                                                 sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = 6371 * c; // Earth's radius in kilometers

  return distance;
}

void readData(Driver drivers[],int numDrivers) { // In summary, the readData function opens a file, reads
                      // driver information from it, and stores the data in an
                      // array of Driver structures. It assumes that the file
                      // follows a specific format where each line contains the
                      // driver's ID, name, latitude, longitude, phone number,
                      // and car number, separated by spaces.
  FILE *infile;
  infile = fopen("drivers.txt", "r");
  if (infile == NULL) {
    fprintf(stderr, "\nError opening file\n");
    exit(1); // check
  }

  int i = 0;
  while (i < numDrivers &&
         fscanf(infile, "%d %s %f %f %s %s %f", &drivers[i].id, drivers[i].name,
                &drivers[i].latitude, &drivers[i].longitude,
                drivers[i].phoneNumber, drivers[i].carNumber,
                &drivers[i].rating) == 7) {
    i++;
  }

  fclose(infile);
}

void readLocations(Location locations[], int numLocations) {
  FILE *infile;
  infile = fopen("locations.txt", "r");
  if (infile == NULL) {
    fprintf(stderr, "\nError opening file\n");
    exit(1);
  }

  int i = 0;
  while (i < numLocations &&
         fscanf(infile, "%d %s %f %f", &locations[i].id, locations[i].name,
                &locations[i].latitude, &locations[i].longitude) == 4) {
    i++;
  }

  fclose(infile);
}

void addEdge(double Distances[MAX_LOCATIONS][MAX_LOCATIONS], int node1,int node2, Location locations[MAX_LOCATIONS]) {
  Distances[node1 - 1][node2 - 1] =
      Haversine(locations[node1 - 1].latitude, locations[node1 - 1].longitude,
                locations[node2 - 1].latitude, locations[node2 - 1].longitude);
  Distances[node2 - 1][node1 - 1] =
      Distances[node1 - 1][node2 - 1]; // For undirected graph
}

void initialiseEdges(double matrix[MAX_LOCATIONS][MAX_LOCATIONS],Location locations[MAX_LOCATIONS]) {
  addEdge(matrix, 0, 1, locations);   // Velachery - Thoraipakkam
  addEdge(matrix, 0, 4, locations);   // Velachery - Madipakkam
  addEdge(matrix, 1, 2, locations);   // Thoraipakkam - Sholinganallur
  addEdge(matrix, 1, 10, locations);  // Thoraipakkam - Medavakkam
  addEdge(matrix, 2, 10, locations);  // Sholinganallur - Medavakkam
  addEdge(matrix, 3, 8, locations);   // Guindy - Saidapet
  addEdge(matrix, 3, 6, locations);   // Guindy - T Nagar
  addEdge(matrix, 8, 6, locations);   // Saidapet - T Nagar
  addEdge(matrix, 6, 12, locations);  // T Nagar - Nungambakkam
  addEdge(matrix, 6, 5, locations);   // T Nagar - Ashok Nagar
  addEdge(matrix, 12, 11, locations); // Nungambakkam - Egmore
  addEdge(matrix, 12, 13, locations); // Nungambakkam - Vadapalani
  addEdge(matrix, 13, 5, locations);  // Vadapalani - Ashok Nagar
  addEdge(matrix, 13, 9, locations);  // Vadapalani - Porur
  addEdge(matrix, 10, 14, locations); // Medavakkam - Tambaram
  addEdge(matrix, 4, 14, locations);  // Madipakkam - Tambaram
  addEdge(matrix, 15, 16, locations); // Chromepet - Pallavaram
  addEdge(matrix, 15, 14, locations); // Chromepet - Tambaram
  addEdge(matrix, 16, 14, locations); // Pallavaram - Tambaram
  addEdge(matrix, 17, 11, locations); // Egmore - Kilpauk
  addEdge(matrix, 17, 18, locations); // Egmore - Royapettah
  addEdge(matrix, 11, 7, locations);  // Kilpauk - Anna Nagar
  addEdge(matrix, 18, 21, locations); // Royapettah - Triplicane
  addEdge(matrix, 18, 22, locations); // Royapettah - Marina Beach
  addEdge(matrix, 21, 22, locations); // Triplicane - Marina Beach
  addEdge(matrix, 19, 20, locations); // Besant Nagar - Adyar
  addEdge(matrix, 19, 22, locations); // Besant Nagar - Marina Beach
  addEdge(matrix, 20, 24, locations); // Mylapore - Adyar
  addEdge(matrix, 20, 21, locations); // Mylapore - Triplicane
  addEdge(matrix, 23, 14, locations); // Kelambakkam - Tambaram
  addEdge(matrix, 23, 2, locations);  // Kelambakkam - Sholinganallur
  addEdge(matrix, 23, 24,
          locations); // Kelambakkam - OMR (Old Mahabalipuram Road)
}

void initGraph(Graph *graph) {
  // Read locations
  graph->numLocations = 25;
  readLocations(graph->locations, graph->numLocations);

  // Read drivers
  graph->numDrivers = 80;
  readData(graph->drivers, graph->numDrivers);

  // Assign cab types to drivers
  int numMini = 20;
  int numSedan = 20;
  int numSuv = 20;
  int numBike = 10;
  int numAuto = 10;
  assignCarTypes(graph->drivers, graph, graph->numDrivers, numMini, numSedan,
                 numSuv, numBike, numAuto);
  // check
  //  Initialize distance matrix
  for (int i = 0; i < MAX_LOCATIONS; i++) {
    for (int j = 0; j < MAX_LOCATIONS; j++) {
      graph->distances[i][j] = INFINITY;
    }
  } // It initializes the distance matrix in the Graph structure. The distance
    // matrix represents the distances between locations in the graph.
    // Initially, all distances are set to INFINITY to indicate that there is no
    // direct connection between locations.

  initialiseEdges(graph->distances, graph->locations);
}

void showlocations(Graph *graph) {
  printf("\n\033[1;47m\033[1;30mAvailable Locations:\033[0m\n\n");
  for (int i = 0; i < graph->numLocations; i++) {
    printf("%d. %s\n", graph->locations[i].id, graph->locations[i].name);
  }
}

void showDrivers(Graph *graph) {
  printf("\n\033[1;47m\033[1;30mDrivers:\033[0m\n\n");
  for (int i = 0; i < graph->numDrivers; i++) {
    printf("%d. %s (%s)\n", graph->drivers[i].id, graph->drivers[i].name,
           graph->drivers[i].carType);
  }
  printf("\n");
}

int least_dist(int dist[], int sptSet[], int V) {
  int min = INT_MAX,
      min_index; // In graph theory, a graph consists of a set of vertices and a
                 // set of edges that connect pairs of vertices.

  for (int v = 0; v < V; v++)
    if (sptSet[v] == 0 && dist[v] <= min)
      min = dist[v], min_index = v;

  return min_index;
}

void YensK(Graph *graph, int src, int dest) {
  int V = graph->numLocations;
  int dist[V];
  int sptSet[V];
  int prev[V];

  for (int i = 0; i < V; i++) {
    dist[i] = INT_MAX;
    sptSet[i] = 0;
    prev[i] = -1;
  }

  dist[src] = 0;
  // YensK's algorithm is being executed to find the shortest path from a source
  // vertex to all other vertices in the graph.
  for (int count = 0; count < V - 1; count++) {
    int u = least_dist(
        dist, sptSet,
        V); // Select the vertex (u) with the minimum distance from the set of
            // vertices not yet included in the shortest path
    sptSet[u] = 1;
    for (int v = 0; v < V; v++) {
      if (!sptSet[v] && graph->distances[u][v] && dist[u] != INT_MAX &&
          dist[u] + graph->distances[u][v] < dist[v]) {
        dist[v] = dist[u] + graph->distances[u][v];
        prev[v] = u;
      }
    }
  }

  if (dist[dest] == INT_MAX) {
    printf("No Route found from %s to %s\n", graph->locations[src].name,
           graph->locations[dest].name);
    return;
  }

  // Construct the path
  int path[MAX_LOCATIONS];
  int pathLength = 0;
  int current = dest;
  while (current != -1) {
    path[pathLength++] = current;
    current = prev[current];
  }

  // Print the path
  printf("Shortest Route details:\n");
  printf("Pickup Location: %s\n", graph->locations[src].name);
  printf("Drop Location: %s\n", graph->locations[dest].name);
  printf("Distance: %d km\n", dist[dest]);
  printf("Route: ");
  for (int i = pathLength - 1; i >= 0; i--) {
    printf("%s", graph->locations[path[i]].name);
    if (i != 0) {
      printf(" -> ");
    }
  }
  printf("\n");
}

float YensKcp(Graph *graph, int src, int dest) {
  int V = graph->numLocations;
  int dist[V];
  int sptSet[V];
  int prev[V];

  for (int i = 0; i < V; i++) {
    dist[i] = INT_MAX;
    sptSet[i] = 0;
    prev[i] = -1;
  }

  dist[src] = 0;

  for (int count = 0; count < V - 1; count++) {
    int u = least_dist(dist, sptSet, V);
    sptSet[u] = 1;
    for (int v = 0; v < V; v++) {
      if (!sptSet[v] && graph->distances[u][v] && dist[u] != INT_MAX &&
          dist[u] + graph->distances[u][v] < dist[v]) {
        dist[v] = dist[u] + graph->distances[u][v];
        prev[v] = u;
      }
    }
  }

  if (dist[dest] == INT_MAX) {
    return dist[dest]; 
  }

  // Construct the path
  int path[MAX_LOCATIONS];
  int pathLength = 0;
  int current = dest;
  while (current != -1) {
    path[pathLength++] = current;
    current = prev[current];
  }
  return dist[dest];
}

int NearestDriver(Graph *graph, int locationIndex, char *CarType) {
  Driver *drivers = NULL;
  int numDrivers = 0;

  if (strcmp(CarType, "mini") == 0) {
    drivers = graph->Mini;
    numDrivers = graph->numMini;
  } else if (strcmp(CarType, "sedan") == 0) {
    drivers = graph->Sedan;
    numDrivers = graph->numSedan;
  } else if (strcmp(CarType, "suv") == 0) {
    drivers = graph->SUV;
    numDrivers = graph->numSuv;
  } else if (strcmp(CarType, "bike") == 0) {
    drivers = graph->Bike;
    numDrivers = graph->numBike;
  } else if (strcmp(CarType, "auto") == 0) {
    drivers = graph->Auto;
    numDrivers = graph->numAuto;
  } else {
    printf("Invalid cab type.\n");
    return -1;
  }

  if (numDrivers == 0) {
    printf("No available drivers for the specified cab type.\n");
    return -1;
  }

  double least_dist = INFINITY;
  int closestDriverIndex = -1;

  for (int i = 0; i < numDrivers; i++) {
    double distance = Haversine(graph->locations[locationIndex].latitude,
                                graph->locations[locationIndex].longitude,
                                drivers[i].latitude, drivers[i].longitude);

    if (distance < least_dist) {
      least_dist = distance;
      closestDriverIndex = i;
    }
  }

  return closestDriverIndex;
}

int isValidCarType(const char *CarType) {
  if (strcmp(CarType, "bike") == 0 || strcmp(CarType, "sedan") == 0 ||
      strcmp(CarType, "mini") == 0 || strcmp(CarType, "suv") == 0 ||
      strcmp(CarType, "auto") == 0) {
    return 1; // Valid cab type
  } else {
    return 0; // Invalid cab type
  }
}

void assigningcab(Graph *graph, int pickupLocation, int dropLocation,char *CarType) {
  int pickupIndex = pickupLocation - 1;
  int dropIndex = dropLocation - 1;

  if (!isValidCarType(CarType)) {
    printf("Invalid cab type.\n");
    return;
  }

  int closestDriver = NearestDriver(graph, pickupIndex, CarType);

  if (closestDriver == -1) {
    printf("No available drivers for the specified cab type.\n");
    return;
  }

  printf("Cab of type '%s' allotted successfully.\n", CarType);
  printf("\n          \033[1;40m\033[1;37m Driver Details: \033[0m\n");

  Driver *driver = NULL;

  // Determine the driver array based on the cab type
  if (strcmp(CarType, "mini") == 0) {
    driver = &(graph->Mini[closestDriver]);
  } else if (strcmp(CarType, "sedan") == 0) {
    driver = &(graph->Sedan[closestDriver]);
  } else if (strcmp(CarType, "suv") == 0) {
    driver = &(graph->SUV[closestDriver]);
  } else if (strcmp(CarType, "bike") == 0) {
    driver = &(graph->Bike[closestDriver]);
  } else if (strcmp(CarType, "auto") == 0) {
    driver = &(graph->Auto[closestDriver]);
  }

  // Calculate distance and ETA to pickup location
  double distanceToPickup =
      Haversine(driver[closestDriver].latitude, driver[closestDriver].longitude,
                graph->locations[pickupIndex].latitude,
                graph->locations[pickupIndex].longitude);

  double averageSpeed = 60.0; // Average speed in km/h
  double etaToPickup =
      (distanceToPickup / averageSpeed) * 60.0; // ETA in minutes

  // Assign ETA to driver
  driver->etaToPickup = etaToPickup;

  // Update the driver's pickup location with the user's drop location
  driver->pickupLocation = dropLocation;

  // Set the driver's availability to 0 (not available)
  driver->availability = 0;

  // Print driver details
  printf("Driver ID: %d\n", driver->id);
  printf("Driver Name: %s\n", driver->name);
  printf("Phone Number: %s\n", driver->phoneNumber);
  printf("Car Number: %s\n", driver->carNumber);
  printf("Car Type: %s\n", CarType);
  printf("Estimated Time of Cab Arrival: %.2f minutes\n", driver->etaToPickup);
  printf("Rating: %.1f\n", driver->rating);
}

void checkCabInput(char *CarType) {
  while (!isValidCarType(CarType)) {
    printf("Confirm the Cab type (bike, auto, suv, mini, sedan): ");
    scanf("%s", CarType);
  }
}

double PeakFee() {
  time_t now;
  time(&now);
  struct tm *timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  int min = timeinfo->tm_min;

  if ((hour == 11 && min >= 30) || (hour >= 12 && hour < 14) ||
      (hour >= 19 && hour < 21)) {
    return 1.2; // Surge fee of 20%
  } else
    return 1.0; // No surge fee
}

int isBookingTimeValid(char *requestedTime) {
  // Get the current time in IST
  time_t rawTime = time(NULL);
  struct tm *currentTime = localtime(&rawTime);

  // Adjust the current time for IST (UTC +5:30)
  currentTime->tm_hour += 5;
  currentTime->tm_min += 30;

  // Handle overflow of minutes
  if (currentTime->tm_min >= 60) {
    currentTime->tm_hour += 1;
    currentTime->tm_min -= 60;
  }

  // Handle overflow of hours
  if (currentTime->tm_hour >= 24) {
    currentTime->tm_hour -= 24;
  }

  // Parse the requested time
  int requestedHour, requestedMinute;
  sscanf(requestedTime, "%d:%d", &requestedHour, &requestedMinute);

  // Calculate the time difference in hours
  int currentHour = currentTime->tm_hour;
  int timeDifference = requestedHour - currentHour;

  // Handle the case when the requested time is on the next day
  if (timeDifference < 0) {
    timeDifference += 24;
  }

  // Check if the time difference is within the valid range (2 to 20 hours)
  if (timeDifference >= 2 && timeDifference <= 20) {
    return 1; // Valid booking time
  } else {
    return 0; // Invalid booking time
  }
}

void assignDriver(char *CarType, driver *drivers, int numDrivers) {
  int i;
  int *availableDriverIndices = NULL;
  int numAvailableDrivers = 0;

  // Get the current time.
  time_t currentTime = time(NULL);

  // Determine the filename based on the CarType
  char filename[100];
  if (strcmp(CarType, "Mini") == 0) {
    strcpy(filename, "Minidrivers.txt");
  } else if (strcmp(CarType, "Sedan") == 0) {
    strcpy(filename, "Sedandrivers.txt");
  } else if (strcmp(CarType, "SUV") == 0) {
    strcpy(filename, "SUVdrivers.txt");
  } else if (strcmp(CarType, "Auto") == 0) {
    strcpy(filename, "Autodrivers.txt");
  } else if (strcmp(CarType, "Bike") == 0) {
    strcpy(filename, "Bikedrivers.txt");
  } else {
    printf("Invalid cab type.\n");
    return;
  }

  // Open the file
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Error opening file.\n");
    return;
  }

  // Read the drivers from the file
  int numFileDrivers;
  fscanf(file, "%d", &numFileDrivers);
  driver *fileDrivers = malloc(numFileDrivers * sizeof(Driver));

  for (i = 0; i < numFileDrivers; i++) {
    fscanf(file, "%s %s %s %s", fileDrivers[i].name, fileDrivers[i].phoneNumber,
           fileDrivers[i].carPlateNumber, fileDrivers[i].CarType);
  }

  fclose(file);

  // Find the available drivers for the requested cab type
  for (i = 0; i < numFileDrivers; i++) {
    if (strcmp(fileDrivers[i].CarType, CarType) == 0) {
      numAvailableDrivers++;
      availableDriverIndices =
          realloc(availableDriverIndices, numAvailableDrivers * sizeof(int));
      availableDriverIndices[numAvailableDrivers - 1] = i;
    }
  }

  // Randomly assign a driver
  if (numAvailableDrivers > 0) {
    int randomIndex = rand() % numAvailableDrivers;
    int assignedDriverIndex = availableDriverIndices[randomIndex];
    printf("\n\n\033[1;47m\033[1;30m ASSIGNED DRIVER DETAILS: \033[0m");
    printf("\nName: %s\n", fileDrivers[assignedDriverIndex].name);
    printf("Phone Number: %s\n", fileDrivers[assignedDriverIndex].phoneNumber);
    printf("Vehicle Nummber: %s\n",
           fileDrivers[assignedDriverIndex].carPlateNumber);
  } else {
    // No drivers available for the requested cab type
    printf("          We are SORRY :-( \n");
    printf("No drivers available for the requested cab type at the moment.\n");
  }

  free(availableDriverIndices);
  free(fileDrivers);
}

Graph graph;

void Admin() {
  initGraph(&graph);
  int choice;
  char input[100];

  while (true) {
    printf("\033[1;96m1.\033[0m ");
    printf("Manage cabs\n");
    printf("\033[1;96m2.\033[0m ");
    printf("Show Locations\n");
    printf("\033[1;96m3.\033[0m ");
    printf("Show Driver's Details");
    printf("\n\033[1;96m4.\033[0m ");
    printf("View Customer's Feedback");
    printf("\n\033[1;96m5.\033[0m ");
    printf("Exit\n\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1: {
      printf("\n\033[1;47m\033[1;30mMANAGE CABS:\033[0m\n\n");
      printf("Manage number of Bikes: ");
      scanf("%d", &n_bike);
      printf("Manage the number of SUVs: ");
      scanf("%d", &n_suv);
      printf("Manage the number of Minis: ");
      scanf("%d", &n_mini);
      printf("Manage the number of Sedans: ");
      scanf("%d", &n_sedan);
      printf("Manage the number of Autos: ");
      scanf("%d", &n_auto);
      
      
      break;
    }
    case 2: {
      printf("\n\n\033[1;96m   -*-*-*-*-*-*-*-*-*-\033[0m\n");
      showlocations(&graph);
      printf("\n\033[1;96m   -*-*-*-*-*-*-*-*-*-\033[0m\n");
      printf("\n");
      break;
    }
    case 3: {
      printf("\n\n\033[1;96m   -*-*-*-*-*-*-*-*-*-\033[0m\n");
      showDrivers(&graph);
      printf("\033[1;96m   -*-*-*-*-*-*-*-*-*-\033[0m\n\n");
      break;
    }
    case 4:{
      readFeedbackFile();
      printf("\n\n");
      break;
    }
    case 5: {
      printf("\n\nExiting...\n\n");
      return;
    }
    default: {
      printf("Invalid choice. Enter again.\n");
      break;
    }
    }
  }
}

void maskPassword(char password[]) {
  struct termios term, oldterm;
  tcgetattr(0, &oldterm);
  term = oldterm;
  term.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(0, TCSANOW, &term);
  int i = 0;
  char ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {
    if (ch == '\b' || ch == '\177') {
      if (i > 0) {
        i--;
        putchar('\b');
        putchar(' ');
        putchar('\b');
        fflush(stdout);
      }
    } else {
      password[i++] = ch;
      putchar('*');
      fflush(stdout);
    }
  }
  password[i] = '\0';
  tcsetattr(0, TCSANOW, &oldterm);
}

bool checkmail(char *email) {
  int i, at = -1, dot = -1;
  int len = strlen(email);
  for (i = 0; i < len; i++) {
    if (email[i] == '@')
      at = i;
    else if (email[i] == '.')
      dot = i;
  }
  if (at == -1 || dot == -1)
    return false;
  if (at > dot)
    return false;
  if (at == 0 || dot == 0 || dot == len - 1)
    return false;
  return true;
}

bool checkPhoneNumber(char *number) {
  int i;
  int len = strlen(number);
  if (len != 10)
    return false;
  for (i = 0; i < len; i++) {
    if (!isdigit(number[i]))
      return false;
  }
  return true;
}

int checkStrength(char password[]) {
  int i, hasUpper = 0, hasLower = 0, hasNumber = 0, hasSpecial = 0;
  int len = strlen(password);
  if (len < 8) {
    printf("Password should be at least 8 characters long.\n");
    return 0;
  }
  for (i = 0; i < len; i++) {
    if (isupper(password[i])) {
      hasUpper = 1;
    } else if (islower(password[i])) {
      hasLower = 1;
    } else if (isdigit(password[i])) {
      hasNumber = 1;
    } else if (ispunct(password[i])) {
      hasSpecial = 1;
    }
  }
  if (!hasUpper) {
    printf("Password should contain at least one uppercase letter.\n");
    return 0;
  }
  if (!hasLower) {
    printf("Password should contain at least one lowercase letter.\n");
    return 0;
  }
  if (!hasNumber) {
    printf("Password should contain at least one number.\n");
    return 0;
  }
  if (!hasSpecial) {
    printf("Password should contain at least one special character.\n");
    return 0;
  }
  return 1;
}

void registerUser() {
  User user, existingUser;
  int usernameExists;
  FILE *fptr;
  do {
    usernameExists = 0;
    printf("\nEnter username: ");
    scanf("%s", user.username);
    getchar();

    fptr = fopen("users.txt", "r");
    if (fptr != NULL) {
      while (fread(&existingUser, sizeof(existingUser), 1, fptr)) {
        if (strcasecmp(existingUser.username, user.username) == 0) {
          usernameExists = 1;
          printf("Username already exists. Please choose a different one.\n");
          break;
        }
      }
      fclose(fptr);
    }
  } while (usernameExists);

  char confirmedPassword[50];

  while (1) {
    printf("\nEnter password: ");
    maskPassword(user.password);
    if (checkStrength(user.password)) {
      while (1) {
        printf("\nConfirm password: ");
        maskPassword(confirmedPassword);
        if (strcmp(user.password, confirmedPassword) == 0) {
          break;
        } else {
          printf("\nPasswords do not match. Please re-enter password.\n");
          printf("\nEnter password: ");
          maskPassword(user.password);
        }
      }
      break;
    }
  }
  sleep(1);
  printf("\n\n\033[1;92mUser Registered \033[0m\n\n");
  sleep(2);
  printf("\n          \033[1;40m\033[1;37mComplete your Profile\033[0m\n\n");
  printf("\nEnter the following details: \n");
  printf("\nEnter your first name: ");
  scanf("%s", user.first_name);
  printf("Enter your last name: ");
  scanf(" %[^\n]s", user.last_name);

  do {
    printf("Enter phone number: ");
    scanf("%s", user.phoneNumber);
    if (checkPhoneNumber(user.phoneNumber)) {
      break;
    } else {
      printf("Invalid phone number. Please enter a valid one.\n");
    }
  } while (1);
  do {
    printf("Enter email: ");
    scanf("%s", user.email);
    if (checkmail(user.email)) {
      break;
    } else {
      printf("\nInvalid email. Please enter a valid one.\n");
    }
  } while (1);

  fptr = fopen("users.txt", "a");
  if (fptr != NULL) {
    fwrite(&user, sizeof(user), 1, fptr);
    fclose(fptr);
    printf("Details stored successfully.\n");
  } else {
    printf("Error occurred while opening the file.\n");
  }
}

int loginUser() {
  User user;
  char username[20], password[20];
  int choice;
  int failedAttempts = 0;
  do {
    sleep(1);
    printf("\nEnter username: ");
    scanf("%s", username);
    getchar();
    strcpy(username_check, username);
    printf("Enter password: ");
    maskPassword(password);
    strcpy(password_check, password);
    FILE *fptr = fopen("users.txt", "r");
    if (fptr != NULL) {
      int loginSuccessful = 0;

      while (fread(&user, sizeof(user), 1, fptr)) {

        if (strcasecmp(user.username, username) == 0 &&
            strcmp(user.password, password) == 0) {
          sleep(2);
          printf("\n\n\033[1;92mLogin Successful \033[0m\n");
          sleep(2);
          loginSuccessful = 1;
          return 1;
        }
      }
      fclose(fptr);
      if (loginSuccessful) {
        return 1;
      } else {
        failedAttempts++;
        if (failedAttempts >= 2) {
          printf(
              "\n\nToo many failed attempts. Locking out for 30 seconds.\n\n");
          int wait_time = 30;
          for (int i = wait_time; i >= 0; i--) {
            printf("%d \r", i);
            fflush(stdout);
            sleep(1);
          }
          printf("\n\033[1;92mLockout Period Expired\033[0m - TRY AGAIN! \n");
          printf("\n");
          failedAttempts = 0;
        } else {
          printf("\n\033[1;31mInvalid username or password\033[0m\n");
          sleep(1);
          printf("\n1. Try again\n2. Register\n");
          printf("\nEnter your choice: ");
          scanf("%d", &choice);
          if (choice == 2) {
            registerUser();
          }
        }
      }
    } else {
      printf("Error occurred while opening the file.\n");
    }
  } while (choice == 1);
  return 0;
}

void FAQ() {
  FILE *file = fopen("cab_faqs.txt", "r");
  if (file == NULL) {
    printf("\033[1;31mERROR opening the file\033[0m\n");
    return;
  }
  char line[256];
  while (fgets(line, sizeof(line), file) != NULL) {
    printf("%s", line);
  }

  fclose(file);
  printf("\n\nReturning to ");
  printf("\033[1;95mUser\033[0m");
  printf(" Portal\n");
  sleep(3);
}

void view_profile() {
  FILE *file = fopen("users.txt", "r");
  if (file == NULL) {
    printf("Error opening the file.\n");
    return;
  }

  User user;
  int found = 0;
  while (fread(&user, sizeof(User), 1, file)) {

    if (strcasecmp(user.username, username_check) == 0 &&
        strcmp(user.password, password_check) == 0) {
      printf("Name: %s %s\n", user.first_name, user.last_name);
      printf("Phone Number: %s\n", user.phoneNumber);
      printf("Email: %s\n", user.email);
      printf("\n");
      found = 1;
      printf("Returning to ");
      printf("\033[1;95mUser\033[0m");
      printf(" Portal\n");
      sleep(3);
      break;
    }
  }

  if (!found) {
    printf("User not found.\n");
  }

  fclose(file);
}

int profile();

void cab_allot() {
  Graph graph;
  initGraph(&graph);
  CarType CarTypes[5];

  strcpy(CarTypes[0].name, "Bike");
  CarTypes[0].fareRate = 7.5;
  CarTypes[0].availability = 1;
  CarTypes[0].fareperkm = 2.5;

  strcpy(CarTypes[1].name, "Auto");
  CarTypes[1].fareRate = 10.0;
  CarTypes[1].availability = 1;
  CarTypes[1].fareperkm = 5.0;

  strcpy(CarTypes[2].name, "SUV");
  CarTypes[2].fareRate = 30.0;
  CarTypes[2].availability = 1;
  CarTypes[2].fareperkm = 20.5;

  strcpy(CarTypes[3].name, "Mini");
  CarTypes[3].fareRate = 15.0;
  CarTypes[3].availability = 1;
  CarTypes[3].fareperkm = 10.5;

  strcpy(CarTypes[4].name, "Sedan");
  CarTypes[4].fareRate = 25.0;
  CarTypes[4].availability = 1;
  CarTypes[4].fareperkm = 12.5;

  int pickupLocation, dropLocation;
  char carType[20];

  int choice = 0;
  while (choice != 5) {
    printf("\n         \033[1;47m\033[1;30m Available Bookings: \033[0m\n\n");
    printf(" 1. Spot Booking\n");
    printf(" 2. Advance Booking\n");
    printf(" 3. Exit\n\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1: {
      showlocations(&graph);
      printf("\nEnter pickup location ID: ");
      scanf("%d", &pickupLocation);
      printf("Enter drop location ID: ");
      scanf("%d", &dropLocation);

      // Display available cab types
      int y = YensKcp(&graph, pickupLocation, dropLocation);
      printf("\n");
      print_availcabs(CarTypes, y);

      printf("\nEnter the cab type (0-4): ");
      int cabChoice;
      scanf("%d", &cabChoice);

      // Validate the cab choice
      while (cabChoice < 0 || cabChoice > 4 ||
             CarTypes[cabChoice].availability == 0) {
        printf("Invalid cab type choice!\n");
        printf("Please enter a valid cab type (0-4): ");
        scanf("%d", &cabChoice);
      }

      strcpy(carType, CarTypes[cabChoice - 1].name);

      checkCabInput(carType); // Validate the cab type input
      float j = printEstimate(CarTypes, y, cabChoice);
      printf("\nEstimated cost: ");
      printf("\033[1;47m\033[1;30m₹%.2f/-\033[0m\n\n", j);

      // Ask for confirmation
      char confirm;
      printf("Confirm allotting the cab? (y/n): ");
      scanf(" %c", &confirm);

      if (confirm == 'y' || confirm == 'Y') {
        printf("\n Finding the shortest Route...\n\n");
        sleep(2);
        YensK(&graph, pickupLocation, dropLocation);
        int e;
        printf("\n\n");
        for (e = 0; e <= 100; e += 5) {
          loadingAnimation(e);
          usleep(300000);
        }
        sleep(2);
        printf("\n\nNearest cab found. Cab is on its way!\n\n\n");
        assigningcab(&graph, pickupLocation, dropLocation, carType);

        printf("\n");
        printf("\n");
        int choice1 = 0;
        printf("Take Action...\n\n");
        printf("1.Cancel the booking!\n");
        printf("2.Continue\n");
        printf("Enter the choice:");
        scanf("%d", &choice1);

        switch (choice1) {
        case 1: {
          printf("\nCab booking cancelled. :(\n");
          float cancellationFee = 0.3 * j;
          printf("\nCancellation Fee: ");
          printf("\033[1;47m\033[1;30m₹%.2f/-\033[0m\n\n", cancellationFee);
          printf("\n          \U0001F917 \033[1;43m\033[1;37m Thank You VISIT "
                 "AGAIN! \033[0m \U0001F917\n\n");
          break;
        }
        case 2: {
          double surgeFee = PeakFee();
          float estimatedCost = printEstimate(CarTypes, y, cabChoice);
          float finalPrice = estimatedCost * surgeFee;
          printf("\n          \033[1;34m INVOICE \033[0m\n");
          printf("\nSurge Fee: %.1f%%\n", (surgeFee - 1.0) * 100.0);
          printf("\nFinal Price: Rs %.2f\n\n", finalPrice);
          // After completing the ride
printf("\n\033[1;40m\033[1;37m Thank you for using our service! We hope you had a comfortable ride. \033[0m\n");

// Ask for feedback

          int feedback;
printf("\n\nPlease rate your ride experience (1-5, 5 being excellent): ");
scanf("%d", &feedback);

// Validate the feedback
while (feedback < 1 || feedback > 5) {
    printf("\nInvalid rating! Please enter a valid rating (1-5): ");
    scanf("%d", &feedback);
}

// Display thank you message based on feedback
if (feedback >= 4) {
    printf("\n\n\033[1;32m Thank you for your positive feedback! We are glad you had a great ride with us. \033[0m\n");
} else if (feedback >= 3) {
    printf("\n\n\033[1;33m Thank you for your feedback! We will work on improving your experience. \033[0m\n");
} else {
    printf("\n\n\033[1;31m We apologize for any inconvenience you faced during the ride. We will make sure to improve. \033[0m\n");
}

// Show farewell message
printf("\n\n\033[1;40m\033[1;37m Have a nice day! \033[0m\n");
;
          FILE *file = fopen("feedback.txt", "a");
if (file != NULL) {
    fprintf(file, "User Feedback: %d\n", feedback);
    fclose(file);
} else {
    printf("Error opening feedback file.\n");
}
          printf("\n          \U0001F917 \033[1;43m\033[1;37m Thank You VISIT "
                 "AGAIN! \033[0m \U0001F917\n\n");
        }
        }
      } else {
        // Ask for further action
        char action;
        printf("What would you like to do?\n");
        printf("1. Cancel cab booking\n");
        printf("2. Reselect cab type\n");
        printf("Enter your choice (1-2): ");
        scanf(" %c", &action);

        while (action != '1' && action != '2') {
          printf("Invalid choice! Please enter a valid choice (1-2): ");
          scanf(" %c", &action);
        }

        if (action == '1') {
          printf("Cab booking cancelled.\n");
          float cancellationFee = 0.3 * j;
          printf("\nCancellation Fee: Rs %.2f\n", cancellationFee);
          printf("\n          \U0001F917 \033[1;43m\033[1;37m Thank You VISIT "
                 "AGAIN! \033[0m \U0001F917\n\n");
        } else {
          printf("\n\n Give us a minute...\n\n");
          sleep(2);
          // Display available cab types and prices again
          print_availcabs(CarTypes, y);

          printf("Enter the cab type (0-4) to book: ");
          scanf("%d", &cabChoice);

          // Validate the cab choice
          while (cabChoice < 0 || cabChoice > 4 ||
                 CarTypes[cabChoice].availability == 0) {
            printf("Invalid cab type choice! Please enter a valid cab type "
                   "(0-4): ");
            scanf("%d", &cabChoice);
          }
          strcpy(carType, CarTypes[cabChoice - 1].name);
          checkCabInput(carType); // Validate the cab type input
          float j = printEstimate(CarTypes, y, cabChoice);
          printf("\nEstimated cost: %.2f\n\n", j);
          printf("\n Finding the shortest Route...\n\n");
          YensK(&graph, pickupLocation, dropLocation);
          assigningcab(&graph, pickupLocation, dropLocation, carType);
          printf("\n");

          double surgeFee = PeakFee();
          float estimatedCost = printEstimate(CarTypes, y, cabChoice);
          float finalPrice = estimatedCost * surgeFee;

          printf("\n          \033[1;34m INVOICE \033[0m\n");
          printf("\n\nSurge Fee: %.1f%%\n\n", (surgeFee - 1.0) * 100.0);
          printf("\nFinal Price: Rs %.2f\n", finalPrice);
          // After completing the ride
printf("\\nn\033[1;40m\033[1;37m Thank you for using our service! We hope you had a comfortable ride. \033[0m\n");

// Ask for feedback
int feedback;
printf("\n\nPlease rate your ride experience (1-5, 5 being excellent): ");
scanf("%d", &feedback);

// Validate the feedback
while (feedback < 1 || feedback > 5) {
    printf("Invalid rating! Please enter a valid rating (1-5): ");
    scanf("%d", &feedback);
}

// Display thank you message based on feedback
if (feedback >= 4) {
    printf("\\nn\033[1;32m Thank you for your positive feedback! We are glad you had a great ride with us. \033[0m\n");
} else if (feedback >= 3) {
    printf("\n\n\033[1;33m Thank you for your feedback! We will work on improving your experience. \033[0m\n");
} else {
    printf("\n\033[1;31m We apologize for any inconvenience you faced during the ride. We will make sure to improve. \033[0m\n");
}

// Show farewell message
printf("\n\n\033[1;40m\033[1;37m Have a nice day! \033[0m\n");
;
FILE *file = fopen("feedback.txt", "a");
if (file != NULL) {
    fprintf(file, "User Feedback: %d\n", feedback);
    fclose(file);
} else {
    printf("Error opening feedback file.\n");
}
          
          printf("\n          \U0001F917 \033[1;43m\033[1;37m Thank You VISIT "
                 "AGAIN! \033[0m \U0001F917\n\n");
        }
      }
      break;
    }

    case 2: {
      showlocations(&graph);
      do {
        printf("Enter pickup location ID (0-24): ");
        if (scanf("%d", &pickupLocation) != 1) {
          printf("Invalid input! Please enter a numeric value.\n");
          while (getchar() != '\n')
            ; // Clear input buffer
          continue;
        }
        if (pickupLocation < 0 || pickupLocation > 24) {
          printf("Invalid pickup location ID! Please enter a value between 0 "
                 "and 24.\n");
        }
      } while (pickupLocation < 0 || pickupLocation > 24);

      do {
        printf("Enter drop location ID (0-24): ");
        if (scanf("%d", &dropLocation) != 1) {
          printf("Invalid input! Please enter a numeric value.\n");
          while (getchar() != '\n')
            ; // Clear input buffer
          continue;
        }
        if (dropLocation < 0 || dropLocation > 24) {
          printf("Invalid drop location ID! Please enter a value between 0 and "
                 "24.\n");
        }
      } while (dropLocation < 0 || dropLocation > 24);
      time_t rawTime = time(NULL);
      struct tm *timeInfo = localtime(&rawTime);

      timeInfo->tm_hour += 5;
      timeInfo->tm_min += 30;

      if (timeInfo->tm_min >= 60) {
        timeInfo->tm_hour += 1;
        timeInfo->tm_min -= 60;
      }

      if (timeInfo->tm_hour >= 24) {
        timeInfo->tm_hour -= 24;
      }

      char timeStr[9];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeInfo);

      printf("Current IST time: %s\n", timeStr);

      BookingRequest request;
      printf("Bike, Auto, SUV, Mini, Sedan\n");
      printf("Enter cab type from above: ");
      scanf("%s", request.CarType);

      printf("Enter booking time (In 24hr Format, HH:MM): ");
      scanf("%s", request.requestedTime);

      if (!isBookingTimeValid(request.requestedTime)) {
        printf("Invalid booking time.The booking time should lay within the "
               "range of 2 to 10 hours on the hour booking.\n");
      }
      printf("\n Finding the shortest Route...\n\n");
      sleep(1.5);
      YensK(&graph, pickupLocation, dropLocation);
      int assignedDriverIndices;
      FILE *file = fopen("drivers.txt", "r");
      if (file == NULL) {
        printf("Error opening file.\n");
      }

      int numDrivers;
      fscanf(file, "%d", &numDrivers);
      driver *drivers = malloc(numDrivers * sizeof(driver));

      int i;
      for (i = 0; i < numDrivers; i++) {
        fscanf(file, "%s %s %s", drivers[i].name, drivers[i].phoneNumber,
               drivers[i].carPlateNumber);
      }

      fclose(file);

      assignDriver(request.CarType, drivers, numDrivers);

      free(drivers);
      printf("\nAdvance Booking Fee for the vehicle type %s is \n",
             request.CarType);
      if (request.CarType == 'Bike') {
        printf("₹30/-");
      } else if (request.CarType == 'Auto') {
        printf("₹40/-");
      } else if (request.CarType == 'SUV') {
        printf("₹70/-");
      } else if (request.CarType == 'Mini') {
        printf("₹50/-");
      } else {
        printf("60");
      }
    }

    case 3: {
      printf("\n\nBack to User Portal...\n");
      sleep(2);
      profile();
      break;
    }

    default: {
      printf("Invalid choice! Please try again.\n");
      break;
    }
    }
  }
}

void Users() {
  sleep(2);
  printf("\n\n          \033[1;45m\033[1;37mWelcome to User Portal!\033[0m\n");
  int choice2;
  int shouldRepeat2 = 1;
  while (shouldRepeat2) {
    printf("  \033[1;95m\n1. \033[0m");
    printf("Register");
    printf("  \033[1;95m\n2. \033[0m");
    printf("Login\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice2);
    switch (choice2) {
    case 1:
      registerUser();
      break;
    case 2: {
      if (loginUser()) {
        profile();
        shouldRepeat2 = 0;
      }
      break;
    }
    default: {
      printf("Invalid choice.\n");
      break;
    }
    }
  }
}

int profile() {
  int shouldRepeat3 = 1, choice3 = 0;
  while (shouldRepeat3) {
    printf("\n\n          \033[1;45m\033[1;37m Welcome back USER! \033[0m "
           "\U0001F44B\n");
    printf("\n1. Display Profile\n");
    printf("2. Request Cab\n");
    printf("3. Help - FAQ's\n");
    printf("4. Logout \n");
    printf("\nEnter your choice: ");
    scanf(" %d", &choice3);
    switch (choice3) {
    case 1: {
      printf("\n");
      printf("\n         \033[1;40m\033[1;37m Your Information \033[0m\n");
      view_profile();
      break;
    }
    case 2: {
      cab_allot();
      shouldRepeat3 = 0;
      break;
    }
    case 3: {
      FAQ();
      break;
    }
    case 4: {
      Users();
      break;
    }
    default: {
      printf("Invalid choice.\n");
      break;
    }
    }
  }
  return 0;
}

int main() {
  int choice1;
  int w;
  char password[15];
  int shouldRepeat1 = 1;
  while (shouldRepeat1) {
    printf("\n");
    printf("    \033[1;43m\033[1;37m - 𝐒𝐎𝐅𝐓𝐖𝐀𝐑𝐄 𝐒𝐘𝐒𝐓𝐄𝐌 𝐅𝐎"
           "𝐑 𝐇𝐀𝐈𝐋𝐈𝐍𝐆 𝐂𝐀𝐁 - \033[0m\n");
    printf("\n\n");
    printf("      __   __    ______\n");
    printf("    --___      /|_||_\\`.__\n");
    printf(" WELCOME!     (   _    _ _\\\n");
    printf("---     -     =`-(_)--(_)-'\n");
    printf("\n\n");
    printf("\033[1;47m\033[1;30mMAIN MENU:\033[0m    ");
    printf("1. ");
    printf("\033[1;96mAdmin\033[0m\n");
    printf("              2. ");
    printf("\033[1;95mUser\033[0m\n");
    printf("              3. ");
    printf("\033[1;90mExit\033[0m\n");
    printf("\nEnter your choice: ");
    scanf("%d", &choice1);
    getchar();
    switch (choice1) {
    case 1: {
      printf("You selected");
      printf("\033[1;96m Admin\033[0m\n");
      do {
        printf("\nEnter password: ");
        maskPassword(password);
        printf("\n");
        if (!strcmp(password, "admin123")) {
          w = 1;
          printf("\n\n            \U0001F490 \033[1;46m\033[1;37m Welcome, "
                 "Admin! \033[0m \U0001F490 \n\n");
          Admin();
          printf("\033[1;96mAdmin\033[0m");
          printf(" Exitting....\n\n");
          printf("Returning to ");
          printf("\033[1;47m\033[1;30mMAIN MENU:\033[0m");
          sleep(1);
          break;
        } else {
          w = 0;
          printf("\nPassword incorrect!  \033[1;31m=(\033[0m\n ");
          printf("\n\033[1;38;2;255;0;0mTry again!\033[0m\n");
        }
      } while (w != 1);
      break;
    }
    case 2: {
      printf("You selected ");
      printf("\033[1;95mUser\033[0m\n");
      Users();
      shouldRepeat1 = 0;
      break;
    }
    case 3: {
      printf("\n\033[1;31mProgram terminated.\033[0m\n");
      shouldRepeat1 = 0;
      break;
    }
    default: {
      printf("Invalid choice. Please try again.\n");
      break;
    }
    }
    printf("\n");
  }
  return 0;
}