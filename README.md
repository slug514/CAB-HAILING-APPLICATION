Cab Booking Software

**Description:**

The cab booking software efficiently assigns cabs to customers based on their requests and locations. It handles pricing factors such as base fare, distance traveled, surge fees during peak demand, advance booking fees, and cancellation fees. The system prioritizes minimizing customer waiting time, assigns the best-rated available drivers, and ensures drivers travel the minimum distance for pickups.

Modules:

**Sign Up/Login Module:**
Supports both User and Admin logins.
User registration with username, password, mobile number, and email.
Admin login with username and password.
Functionalities include managing cab numbers, displaying locations, driver details, and customer feedback.

**Booking Module:**
Allows spot booking and advance booking.
Users select pick-up and drop locations, and vehicle type.
Provides estimated costs and confirmation options.

**Cab Allocation Module:**
Allocates the best driver based on distance and rating.
Utilizes Yen’s-K algorithm for shortest paths.
Calculates distances using Haversine’s Formula.

**Billing and Feedback Module:**
Calculates ride fare based on distance and vehicle type.
Applies surge fees during peak hours.
Users can provide feedback (rating) for drivers, affecting their average rating.
