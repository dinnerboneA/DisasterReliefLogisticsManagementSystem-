# DisasterReliefLogisticsManagementSystem-
Disaster Relief Logistics Management System is a C++ system that models a disaster relief logistics operation, managing volunteers, supplies, emergency responses, and transportation schedules. The system must utilize Stacks, Queues, Priority Queues, and Circular Queues, with each group member assigned a specific role and set of functionalities.


Disaster Relief Logistics Management System requires handling multiple components efficiently, including:
•	Managing emergency supply movement
•	Coordinating volunteer dispatch
•	Prioritizing urgent aid requests
•	Scheduling transport vehicles for continuous operations


Role 1: SUPPLY BASE MANAGER
Responsible for managing the dispatch of supply packages from the base to the field.
Functionalities:
1.	Pack Supply Box: Add a new supply box with details (e.g., ID, type, quantity).
2.	Send Supply Package: Remove and dispatch one supply package for delivery.
3.	View Packed Supplies: Display all supply boxes waiting for dispatch.


Role 2: VOLUNTEER OPERATIONS OFFICER
Handles the orderly registration and field deployment of volunteer workers.
Functionalities:
1.	Register Volunteer: Accept new volunteer registration (name, contact, skill area).
2.	Deploy Volunteer to Field: Assign a volunteer to a relief site.
3.	View Registered Volunteers: Display all available volunteers yet to be deployed.

   
Role 3: EMERGENCY REQUEST COORDINATOR
Processes aid requests from different relief sites with varying urgency levels.
Functionalities:
1.	Log Emergency Request: Add a new request with location, type (e.g., medical, food), and urgency level.
2.	Process Most Critical Request: Identify and respond to the most pressing request.
3.	View Pending Requests: Display all pending requests with their details.

   
Role 4: TRANSPORT UNIT SCHEDULER	
Manages the rotation and availability of transportation vehicles used for relief efforts.
Functionalities:
1.	Add Vehicle to Schedule: Register a vehicle in the active duty schedule.
2.	Rotate Vehicle Shift: Rotate the vehicles so each gets equal usage over time.
3.	Display Vehicle Schedule: Show the current order and availability of transport units.

   
