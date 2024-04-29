PS: Develop a cab booking software that efficiently assigns cabs to customers based on their requests and locations. The system handles pricing factors like base fare, distance traveled, surge fees during peak demand, advance booking fees, and cancellation fees. 
It prioritizes minimizing customer waiting time, assigns the best-rated available drivers, and ensures drivers travel the minimum distance for pickups.


DESCRIPTION OF EACH MODULE:

(i) SIGN UP/LOGIN MODULE: -Two types of login are considered- Admin Login and User Login
USER LOGIN: - User will be asked to choose between Register(if not registered before) and Login. -If Register option is selected, user will be prompted to enter username and
password(with double verification and character specific rules such as password
should be minimum 8 characters long and should have a special character etc.). Certain information about the user such as mobile number and e-mail address of the
user will be asked for future use. -If Login option is selected, user will be prompted to enter username and password.
It will be compared with the available user information, and if both matches, user
will be logged in successfully. After logging in, user can see options such as
- Display Profile: displays stored user information
- Request Cab: for booking cab
- Help-FAQ’s: displays basic FAQ’s which will help the user in understanding
the application better
- Logout
ADMIN LOGIN: -Admin will be prompted to enter username and admin password. If admin password
is right, user will be logged in as admin successfully. After logging in, admin can see options such as
- Manage number of cabs: It allows the admin to change the number of
vehicles under each type
- Show Locations: Shows all pre-defined locations along with their latitudes
and longitudes - Show Driver’s Details: Shows all driver details who are
currently active
- View Customer’s Feedback: Shows previously recorded customer feedbacks
- Exit
- 
(ii) BOOKING MODULE: - User will be asked to choose between Spot Booking and Advance Booking
- All available locations with the location ID will be displayed. - User will be prompted to enter pick up location and drop location
SPOT BOOKING: . - Users will now be asked to choose the type of vehicle they want to travel in
through double verification
- Final estimated cost will be returned and the confirmation will be asked from the
user. - If selected yes, cab allocation will take place. ADVANCE BOOKING: - Current time will be displayed and the time in which the user wants to travel will be
asked from the user. - Cab allocation will take place.

(iii) CAB ALLOCATION MODULE:
Cab allocation module allocates the best driver for the ride. It choose a driver, in a
such a way that distance from the driver’s current location and the user’s current
location(ride start location) is the least and at the same time the rating of the driver
is high. Shortest distance is calculated using
Yen’s-K algorithm. In Yen’s-K algorithm, the edge weights between nodes are
calculated using
Haversine Distance Formula. Yen’s-K Algorithm:
Yen’s Shortest Path algorithm computes a number of shortest paths between two
nodes. The algorithm is often referred to as Yen’s k-Shortest Path algorithm, where k
is the number of shortest paths to compute. The algorithm supports weighted
graphs with positive relationship weights(only non-negative edge costs are
considered). It also respects parallel relationships between the same two nodes
when computing multiple shortest paths. For k = 1, the algorithm behaves exactly like Dijkstra’s shortest path algorithm and
returns the shortest path. For k = 2, the algorithm returns the shortest path and the
second shortest path between the same source and target node. Generally, for k = n, the algorithm computes at most n paths which are discovered in the order of their
total cost.

Haversine’s Formula:
The Haversine formula calculates the shortest distance between two points on a
sphere using their latitudes and longitudes measured along the surface. It is
important for use in navigation. where r is the radius of the earth(6371 km), d is the distance between two
points, is the latitude of the two points, and is the longitude of the
two points respectively.

(iv) BILLING MODULE AND FEEDBACK MODULE: - Ride fare is calculated based on the following criteria. - For the first 3 kilometers, a base price is charged which depends on the type
of vehicle. - For >3 kilometers, after 3 kilometers, price is calculated using cost per km. - Bill= Base fare+ (Total kms-3)*cost per km
- Surge of 20% is also applied if ride is to be taken in certain pre-defined time
ranges. - User is asked to enter feedback (rating on a scale of 1-5) which will later be
reflected on the driver’s rating by taking the average of previous rating and current
feedback rating. This feedback will also be stored in a file without user information.
