#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <climits>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

// Vehicle structure
struct Vehicle {
    string id;
    string type;
    string status;
    string lastUsed;
    int use_age;
    int assignedEmergencyId; // Track which emergency it's assigned to
    Vehicle* next; // For circular linked list implementation
    
    Vehicle(string vehId, string vehType, string stat = "AVAILABLE", string lastUse = "", int usage = 0, int emergencyId = -1) {
        id = vehId;
        type = vehType;
        status = stat;
        lastUsed = lastUse;
        use_age = usage;
        assignedEmergencyId = emergencyId;
        next = nullptr;
    }
    
    // Function to get current timestamp
    static string getCurrentTimestamp() {
        time_t now = time(0);
        tm* timeinfo = localtime(&now);
        
        stringstream ss;
        ss << (timeinfo->tm_year + 1900) << "-"
           << setfill('0') << setw(2) << (timeinfo->tm_mon + 1) << "-"
           << setfill('0') << setw(2) << timeinfo->tm_mday << "T"
           << setfill('0') << setw(2) << timeinfo->tm_hour << ":"
           << setfill('0') << setw(2) << timeinfo->tm_min << ":"
           << setfill('0') << setw(2) << timeinfo->tm_sec << "Z";
        
        return ss.str();
    }
};

// Circular Queue class for Vehicle Rotation
class VehicleCircularQueue {
private:
    Vehicle* rear;
    int size;
    
public:
    VehicleCircularQueue() : rear(nullptr), size(0) {}
    
    bool isEmpty() {
        return rear == nullptr;
    }
    
    void enqueue(const Vehicle& vehicle) {
        Vehicle* newVehicle = new Vehicle(vehicle.id, vehicle.type, vehicle.status, 
                                        vehicle.lastUsed, vehicle.use_age, vehicle.assignedEmergencyId);
        
        if (isEmpty()) {
            rear = newVehicle;
            rear->next = rear; // Point to itself
        } else {
            newVehicle->next = rear->next;
            rear->next = newVehicle;
            rear = newVehicle;
        }
        size++;
    }
    
    Vehicle getNextAvailableByUseAge() {
        if (isEmpty()) {
            throw runtime_error("No vehicles in queue");
        }
        
        Vehicle* current = rear->next; // Start from front
        Vehicle* lowestUseAgeVehicle = nullptr;
        int lowestUseAge = INT_MAX;
        int checked = 0;
        
        // Find available vehicle with lowest use_age
        while (checked < size) {
            if (current->status == "AVAILABLE" && current->use_age < lowestUseAge) {
                lowestUseAge = current->use_age;
                lowestUseAgeVehicle = current;
            }
            current = current->next;
            checked++;
        }
        
        if (lowestUseAgeVehicle == nullptr) {
            throw runtime_error("No available vehicles");
        }
        
        return *lowestUseAgeVehicle;
    }
    
    Vehicle getNextAvailable() {
        return getNextAvailableByUseAge(); // Use the new method
    }
    
    void assignVehicle(const string& vehicleId, int emergencyId) {
        if (isEmpty()) return;
        
        Vehicle* current = rear->next;
        int checked = 0;
        
        while (checked < size) {
            if (current->id == vehicleId) {
                current->status = "IN_USE";
                current->use_age++;
                current->lastUsed = Vehicle::getCurrentTimestamp();
                current->assignedEmergencyId = emergencyId;
                break;
            }
            current = current->next;
            checked++;
        }
    }
    
    void returnVehicle(const string& vehicleId) {
        if (isEmpty()) return;
        
        Vehicle* current = rear->next;
        int checked = 0;
        
        while (checked < size) {
            if (current->id == vehicleId && current->status == "IN_USE") {
                current->status = "AVAILABLE";
                current->assignedEmergencyId = -1;
                
                // Move this vehicle to the back of the queue for fair rotation
                if (size > 1 && current != rear) {
                    // Find the vehicle before current
                    Vehicle* prev = rear;
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    
                    // Remove current from its position
                    prev->next = current->next;
                    if (current == rear->next) {
                        // If it was at front, no need to change rear
                    }
                    
                    // Add to back
                    current->next = rear->next;
                    rear->next = current;
                    rear = current;
                }
                break;
            }
            current = current->next;
            checked++;
        }
    }
    
    vector<Vehicle> getInUseVehicles() {
        vector<Vehicle> inUseList;
        if (isEmpty()) return inUseList;
        
        Vehicle* current = rear->next;
        int checked = 0;
        
        while (checked < size) {
            if (current->status == "IN_USE") {
                inUseList.push_back(*current);
            }
            current = current->next;
            checked++;
        }
        
        return inUseList;
    }
    
    Vehicle getOldestInUseVehicle() {
        vector<Vehicle> inUseVehicles = getInUseVehicles();
        if (inUseVehicles.empty()) {
            throw runtime_error("No vehicles in use");
        }
        
        Vehicle* oldestVehicle = &inUseVehicles[0];
        for (size_t i = 1; i < inUseVehicles.size(); i++) {
            if (inUseVehicles[i].lastUsed < oldestVehicle->lastUsed) {
                oldestVehicle = &inUseVehicles[i];
            }
        }
        
        return *oldestVehicle;
    }
    
    void display() {
        if (isEmpty()) {
            cout << "No vehicles in schedule." << endl;
            return;
        }
        
        Vehicle* current = rear->next; // Start from front
        int position = 1;
        int checked = 0;
        
        while (checked < size) {
            cout << position << ". " << current->id << "\t\t" 
                 << current->type << "\t\t" 
                 << current->status << "\t\t"
                 << current->use_age << "\t\t"
                 << current->lastUsed << endl;
            current = current->next;
            position++;
            checked++;
        }
    }
    
    int getSize() {
        return size;
    }
    
    ~VehicleCircularQueue() {
        if (!isEmpty()) {
            Vehicle* current = rear->next;
            while (current != rear) {
                Vehicle* temp = current;
                current = current->next;
                delete temp;
            }
            delete rear;
        }
    }
};

// Transport Unit Manager class
class TransportUnitManager {
private:
    VehicleCircularQueue ambulanceQueue;
    VehicleCircularQueue transportCarQueue;  
    VehicleCircularQueue transportVanQueue;
    int vehicleCounter;
    
public:
    TransportUnitManager() {
        vehicleCounter = 115; // Starting from VH115 (will increment for new vehicles)
    }
    
    // Load vehicles from JSON data
    void loadFromJSON(const json& data) {
        // Find highest ID number to set counter
        int maxId = 0;
        for (const auto& v : data["vehicles"]) {
            string id = v["id"];
            int idNum = stoi(id.substr(2)); // Remove 'VH' and convert to int
            if (idNum > maxId) {
                maxId = idNum;
            }
        }
        vehicleCounter = maxId;
        
        // Load vehicles into appropriate circular queues
        for (const auto& v : data["vehicles"]) {
            Vehicle vehicle(v["id"], v["type"], v["status"], v["lastUsed"], v["use_age"]);
            
            if (v["type"] == "Ambulance") {
                ambulanceQueue.enqueue(vehicle);
            } else if (v["type"] == "Transport_Car") {
                transportCarQueue.enqueue(vehicle);
            } else if (v["type"] == "Transport_Van") {
                transportVanQueue.enqueue(vehicle);
            }
        }
    }
    
    // Add new vehicle
    void addVehicle(json& data) {
        cout << "\n=== ADD VEHICLE ===" << endl;
        
        string type;
        int typeChoice;
        
        cout << "Select Vehicle Type:" << endl;
        cout << "1. Ambulance" << endl;
        cout << "2. Transport_Car" << endl;
        cout << "3. Transport_Van" << endl;
        cout << "Enter choice: ";
        cin >> typeChoice;
        
        switch (typeChoice) {
            case 1: type = "Ambulance"; break;
            case 2: type = "Transport_Car"; break;
            case 3: type = "Transport_Van"; break;
            default:
                cout << "\n Invalid vehicle type! Operation cancelled." << endl;
                return;
        }
        
        vehicleCounter++;
        string newId = "VH" + to_string(vehicleCounter);
        
        Vehicle newVehicle(newId, type);
        
        // Add to appropriate circular queue
        if (type == "Ambulance") {
            ambulanceQueue.enqueue(newVehicle);
        } else if (type == "Transport_Car") {
            transportCarQueue.enqueue(newVehicle);
        } else if (type == "Transport_Van") {
            transportVanQueue.enqueue(newVehicle);
        }
        
        // Add to JSON data
        json newVehicleJson = {
            {"id", newId},
            {"type", type},
            {"status", "AVAILABLE"},
            {"lastUsed", newVehicle.lastUsed},
            {"use_age", 0}
        };
        data["vehicles"].push_back(newVehicleJson);
        
        cout << "\n✓ Vehicle added successfully!" << endl;
        cout << "Vehicle ID: " << newId << endl;
        cout << "Type: " << type << endl;
        cout << "Status: AVAILABLE" << endl;
        cout << "Use Age: 0" << endl;
        cout << "Last Used: " << newVehicle.lastUsed << endl;
    }
    
    // Get vehicle requirements for emergency type
    struct VehicleRequirement {
        int ambulance;
        int transport_car;
        int transport_van;
    };
    
    VehicleRequirement getVehicleRequirements(const string& emergencyType) {
        if (emergencyType == "Medical") {
            return {1, 1, 0}; // 1 Ambulance + 1 Transport_Car
        } else if (emergencyType == "Earthquake") {
            return {2, 1, 1}; // 2 Ambulance + 1 Transport_Car + 1 Transport_Van
        } else if (emergencyType == "Search & Rescue") {
            return {0, 2, 0}; // 2 Transport_Car
        } else if (emergencyType == "Flood") {
            return {2, 2, 1}; // 2 Ambulance + 2 Transport_Car + 1 Transport_Van
        }
        return {0, 0, 0};
    }
    
    // Check if we have enough vehicles for emergency type
    bool checkVehicleAvailability(const string& emergencyType) {
        VehicleRequirement req = getVehicleRequirements(emergencyType);
        
        int availableAmbulance = ambulanceQueue.getSize() - ambulanceQueue.getInUseVehicles().size();
        int availableTransportCar = transportCarQueue.getSize() - transportCarQueue.getInUseVehicles().size();
        int availableTransportVan = transportVanQueue.getSize() - transportVanQueue.getInUseVehicles().size();
        
        return availableAmbulance >= req.ambulance && 
               availableTransportCar >= req.transport_car && 
               availableTransportVan >= req.transport_van;
    }
    
    // Get highest priority emergency request that needs vehicle (and doesn't already have vehicles)
    json getEmergencyNeedingVehicle(const json& data) {
        json highestRequest = nullptr;
        int highestUrgency = 0;
        string earliestTime = "";
        
        for (const auto& request : data["emergencyRequests"]) {
            if (request["status"] == "READY_FOR_ASSIGNMENT" && request["vehicle_Id"].empty()) {
                int urgency = request["urgencyLevel"];
                string timestamp = request["timestamp"];
                
                if (urgency > highestUrgency || 
                    (urgency == highestUrgency && (earliestTime.empty() || timestamp < earliestTime))) {
                    highestRequest = request;
                    highestUrgency = urgency;
                    earliestTime = timestamp;
                }
            }
        }
        
        return highestRequest;
    }
    
    // Assign vehicles to emergency based on type requirements
    void assignVehicleToEmergency(json& data) {
        cout << "\n=== ASSIGN VEHICLE TO EMERGENCY ===" << endl;
        
        // Get emergency needing vehicle
        json emergencyRequest = getEmergencyNeedingVehicle(data);
        
        if (emergencyRequest == nullptr) {
            cout << "\n No emergency requests need vehicle assignment!" << endl;
            return;
        }
        
        int emergencyId = emergencyRequest["id"];
        string location = emergencyRequest["location"];
        string emergencyType = emergencyRequest["requestType"];
        int urgency = emergencyRequest["urgencyLevel"];
        
        cout << "\nEmergency Assignment Details:" << endl;
        cout << "Emergency ID: " << emergencyId << endl;
        cout << "Location: " << location << endl;
        cout << "Type: " << emergencyType << endl;
        cout << "Urgency: " << urgency << endl;
        
        // Check vehicle availability
        if (!checkVehicleAvailability(emergencyType)) {
            VehicleRequirement req = getVehicleRequirements(emergencyType);
            cout << "\n Not enough vehicles available for this emergency type!" << endl;
            cout << "Required vehicles for " << emergencyType << ":" << endl;
            cout << "- Ambulance: " << req.ambulance << " (Available: " << (ambulanceQueue.getSize() - ambulanceQueue.getInUseVehicles().size()) << ")" << endl;
            cout << "- Transport_Car: " << req.transport_car << " (Available: " << (transportCarQueue.getSize() - transportCarQueue.getInUseVehicles().size()) << ")" << endl;
            cout << "- Transport_Van: " << req.transport_van << " (Available: " << (transportVanQueue.getSize() - transportVanQueue.getInUseVehicles().size()) << ")" << endl;
            return;
        }
        
        // Get requirements and assign vehicles
        VehicleRequirement req = getVehicleRequirements(emergencyType);
        vector<string> assignedVehicleIds;
        
        cout << "\n🚛 Assigning Vehicles:" << endl;
        
        // Assign Ambulances (using lowest use_age first)
        for (int i = 0; i < req.ambulance; i++) {
            try {
                Vehicle ambulance = ambulanceQueue.getNextAvailableByUseAge();
                ambulanceQueue.assignVehicle(ambulance.id, emergencyId);
                assignedVehicleIds.push_back(ambulance.id);
                cout << "- Ambulance: " << ambulance.id << " (Use Age: " << ambulance.use_age << " → " << (ambulance.use_age + 1) << ")" << endl;
                
                // Update JSON - vehicle data
                for (auto& v : data["vehicles"]) {
                    if (v["id"] == ambulance.id) {
                        v["status"] = "IN_USE";
                        v["use_age"] = v["use_age"].get<int>() + 1;
                        v["lastUsed"] = Vehicle::getCurrentTimestamp();
                        break;
                    }
                }
            } catch (const runtime_error& e) {
                cout << " Error assigning ambulance: " << e.what() << endl;
                return;
            }
        }
        
        // Assign Transport_Cars (using lowest use_age first)
        for (int i = 0; i < req.transport_car; i++) {
            try {
                Vehicle car = transportCarQueue.getNextAvailableByUseAge();
                transportCarQueue.assignVehicle(car.id, emergencyId);
                assignedVehicleIds.push_back(car.id);
                cout << "- Transport_Car: " << car.id << " (Use Age: " << car.use_age << " → " << (car.use_age + 1) << ")" << endl;
                
                // Update JSON - vehicle data
                for (auto& v : data["vehicles"]) {
                    if (v["id"] == car.id) {
                        v["status"] = "IN_USE";
                        v["use_age"] = v["use_age"].get<int>() + 1;
                        v["lastUsed"] = Vehicle::getCurrentTimestamp();
                        break;
                    }
                }
            } catch (const runtime_error& e) {
                cout << " Error assigning transport car: " << e.what() << endl;
                return;
            }
        }
        
        // Assign Transport_Vans (using lowest use_age first)
        for (int i = 0; i < req.transport_van; i++) {
            try {
                Vehicle van = transportVanQueue.getNextAvailableByUseAge();
                transportVanQueue.assignVehicle(van.id, emergencyId);
                assignedVehicleIds.push_back(van.id);
                cout << "- Transport_Van: " << van.id << " (Use Age: " << van.use_age << " → " << (van.use_age + 1) << ")" << endl;
                
                // Update JSON - vehicle data
                for (auto& v : data["vehicles"]) {
                    if (v["id"] == van.id) {
                        v["status"] = "IN_USE";
                        v["use_age"] = v["use_age"].get<int>() + 1;
                        v["lastUsed"] = Vehicle::getCurrentTimestamp();
                        break;
                    }
                }
            } catch (const runtime_error& e) {
                cout << " Error assigning transport van: " << e.what() << endl;
                return;
            }
        }
        
        // Update JSON - emergency request (DON'T change status yet)
        for (auto& request : data["emergencyRequests"]) {
            if (request["id"] == emergencyId) {
                request["vehicle_Id"] = assignedVehicleIds;
                
                // Only change status to FULLY_DISPATCHED if ALL requirements are met
                bool hasSupplies = !request["Supply_Id"].empty();
                bool hasVolunteers = !request["Volunteer_Id"].empty();
                bool hasVehicles = !request["vehicle_Id"].empty();
                
                if (hasSupplies && hasVolunteers && hasVehicles) {
                    request["status"] = "FULLY_DISPATCHED";
                }
                // Don't change status if not all requirements are met - keep READY_FOR_ASSIGNMENT
                break;
            }
        }
        
        cout << "\n✓ All vehicles assigned successfully!" << endl;
        cout << "Total vehicles dispatched: " << assignedVehicleIds.size() << endl;
        cout << "Vehicle IDs: ";
        for (size_t i = 0; i < assignedVehicleIds.size(); i++) {
            cout << assignedVehicleIds[i];
            if (i < assignedVehicleIds.size() - 1) cout << ", ";
        }
        cout << endl;
    }
    
    // Return vehicle from mission by type (user selects type, returns first-used of that type)
    void returnVehicleFromMission(json& data) {
        cout << "\n=== RETURN VEHICLE FROM MISSION ===" << endl;
        
        string type;
        int typeChoice;
        
        cout << "Select Vehicle Type to Return:" << endl;
        cout << "1. Ambulance" << endl;
        cout << "2. Transport_Car" << endl;
        cout << "3. Transport_Van" << endl;
        cout << "Enter choice: ";
        cin >> typeChoice;
        
        switch (typeChoice) {
            case 1: type = "Ambulance"; break;
            case 2: type = "Transport_Car"; break;
            case 3: type = "Transport_Van"; break;
            default:
                cout << "\n Invalid vehicle type!" << endl;
                return;
        }
        
        // Get oldest (first-used) vehicle of selected type
        Vehicle* vehicleToReturn = nullptr;
        bool found = false;
        
        try {
            if (type == "Ambulance") {
                Vehicle temp = ambulanceQueue.getOldestInUseVehicle();
                vehicleToReturn = new Vehicle(temp.id, temp.type, temp.status, temp.lastUsed, temp.use_age, temp.assignedEmergencyId);
                found = true;
            } else if (type == "Transport_Car") {
                Vehicle temp = transportCarQueue.getOldestInUseVehicle();
                vehicleToReturn = new Vehicle(temp.id, temp.type, temp.status, temp.lastUsed, temp.use_age, temp.assignedEmergencyId);
                found = true;
            } else if (type == "Transport_Van") {
                Vehicle temp = transportVanQueue.getOldestInUseVehicle();
                vehicleToReturn = new Vehicle(temp.id, temp.type, temp.status, temp.lastUsed, temp.use_age, temp.assignedEmergencyId);
                found = true;
            }
        } catch (const runtime_error& e) {
            cout << "\n No " << type << " vehicles are currently in use!" << endl;
            return;
        }
        
        if (!found || vehicleToReturn == nullptr) {
            cout << "\n No " << type << " vehicles are currently in use!" << endl;
            return;
        }
        
        cout << "\n🚛 Returning Vehicle (First Used of Type):" << endl;
        cout << "Vehicle ID: " << vehicleToReturn->id << endl;
        cout << "Type: " << vehicleToReturn->type << endl;
        cout << "Emergency ID: " << vehicleToReturn->assignedEmergencyId << endl;
        cout << "Use Age: " << vehicleToReturn->use_age << endl;
        cout << "Sent Time: " << vehicleToReturn->lastUsed << endl;
        
        // Return vehicle from appropriate queue
        if (type == "Ambulance") {
            ambulanceQueue.returnVehicle(vehicleToReturn->id);
        } else if (type == "Transport_Car") {
            transportCarQueue.returnVehicle(vehicleToReturn->id);
        } else if (type == "Transport_Van") {
            transportVanQueue.returnVehicle(vehicleToReturn->id);
        }
        
        // Update JSON
        for (auto& v : data["vehicles"]) {
            if (v["id"] == vehicleToReturn->id) {
                v["status"] = "AVAILABLE";
                break;
            }
        }
        
        cout << "\n✓ Vehicle returned successfully!" << endl;
        cout << "Status: IN_USE → AVAILABLE" << endl;
        cout << "Vehicle moved to back of " << type << " rotation queue" << endl;
        
        // Clean up
        delete vehicleToReturn;
    }
    
    // Display vehicle schedule
    void displayVehicleSchedule() {
        cout << "\n=== VEHICLE ROTATION SCHEDULE ===" << endl;
        
        cout << "\n--- AMBULANCE QUEUE ---" << endl;
        cout << "Pos.\tID\t\tType\t\tStatus\t\tUse Age\tLast Used" << endl;
        cout << "============================================================================" << endl;
        if (ambulanceQueue.isEmpty()) {
            cout << "No Ambulance vehicles in schedule." << endl;
        } else {
            ambulanceQueue.display();
        }
        
        cout << "\n--- TRANSPORT_CAR QUEUE ---" << endl;
        cout << "Pos.\tID\t\tType\t\tStatus\t\tUse Age\tLast Used" << endl;
        cout << "============================================================================" << endl;
        if (transportCarQueue.isEmpty()) {
            cout << "No Transport_Car vehicles in schedule." << endl;
        } else {
            transportCarQueue.display();
        }
        
        cout << "\n--- TRANSPORT_VAN QUEUE ---" << endl;
        cout << "Pos.\tID\t\tType\t\tStatus\t\tUse Age\tLast Used" << endl;
        cout << "============================================================================" << endl;
        if (transportVanQueue.isEmpty()) {
            cout << "No Transport_Van vehicles in schedule." << endl;
        } else {
            transportVanQueue.display();
        }
        
        cout << "\nSchedule Summary:" << endl;
        cout << "Ambulance: " << ambulanceQueue.getSize() << " vehicles (" << ambulanceQueue.getInUseVehicles().size() << " in use)" << endl;
        cout << "Transport_Car: " << transportCarQueue.getSize() << " vehicles (" << transportCarQueue.getInUseVehicles().size() << " in use)" << endl;
        cout << "Transport_Van: " << transportVanQueue.getSize() << " vehicles (" << transportVanQueue.getInUseVehicles().size() << " in use)" << endl;
        cout << "\nNote: Each vehicle type rotates in its own circular queue for fair usage" << endl;
    }
};

// Emergency Request structure
struct EmergencyRequest {
    int id;
    string location;
    string requestType;
    int urgencyLevel;
    string status;
    vector<string> Volunteer_Id;
    vector<string> Supply_Id;
    vector<string> vehicle_Id;
    string timestamp;
    EmergencyRequest* next; // For linked list implementation (acting as priority queue)
    
    EmergencyRequest(int reqId, string loc, string type, int urgency, string stat = "PENDING", string time = "") {
        id = reqId;
        location = loc;
        requestType = type;
        urgencyLevel = urgency;
        status = stat;
        Volunteer_Id = {};
        Supply_Id = {};
        vehicle_Id = {};
        timestamp = time.empty() ? getCurrentTimestamp() : time;
        next = nullptr;
    }
    
    // Function to get current timestamp
    static string getCurrentTimestamp() {
        time_t now = time(0);
        tm* timeinfo = localtime(&now);
        
        stringstream ss;
        ss << (timeinfo->tm_year + 1900) << "-"
           << setfill('0') << setw(2) << (timeinfo->tm_mon + 1) << "-"
           << setfill('0') << setw(2) << timeinfo->tm_mday << "T"
           << setfill('0') << setw(2) << timeinfo->tm_hour << ":"
           << setfill('0') << setw(2) << timeinfo->tm_min << ":"
           << setfill('0') << setw(2) << timeinfo->tm_sec << "Z";
        
        return ss.str();
    }
};

// Priority Queue class for Emergency Requests (highest priority first)
class EmergencyPriorityQueue {
private:
    EmergencyRequest* front;
    
public:
    EmergencyPriorityQueue() : front(nullptr) {}
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    // Insert based on priority (urgency level + FIFO for same urgency)
    void enqueue(const EmergencyRequest& request) {
        EmergencyRequest* newRequest = new EmergencyRequest(request.id, request.location, request.requestType, 
                                                           request.urgencyLevel, request.status, request.timestamp);
        newRequest->Volunteer_Id = request.Volunteer_Id;
        newRequest->Supply_Id = request.Supply_Id;
        newRequest->vehicle_Id = request.vehicle_Id;
        
        // If queue is empty or new request has higher priority
        if (front == nullptr || request.urgencyLevel > front->urgencyLevel ||
            (request.urgencyLevel == front->urgencyLevel && request.timestamp < front->timestamp)) {
            newRequest->next = front;
            front = newRequest;
        } else {
            // Find correct position based on priority
            EmergencyRequest* current = front;
            while (current->next != nullptr && 
                   (current->next->urgencyLevel > request.urgencyLevel ||
                    (current->next->urgencyLevel == request.urgencyLevel && current->next->timestamp <= request.timestamp))) {
                current = current->next;
            }
            newRequest->next = current->next;
            current->next = newRequest;
        }
    }
    
    EmergencyRequest dequeue() {
        if (isEmpty()) {
            throw runtime_error("Priority Queue is empty");
        }
        EmergencyRequest* temp = front;
        EmergencyRequest request = *temp;
        front = front->next;
        delete temp;
        return request;
    }
    
    EmergencyRequest peek() {
        if (isEmpty()) {
            throw runtime_error("Priority Queue is empty");
        }
        return *front;
    }
    
    int count() {
        int cnt = 0;
        EmergencyRequest* current = front;
        while (current) {
            cnt++;
            current = current->next;
        }
        return cnt;
    }
    
    void display() {
        EmergencyRequest* current = front;
        while (current) {
            cout << current->id << "\t\t" 
                 << current->location << "\t\t" 
                 << current->requestType << "\t\t" 
                 << current->urgencyLevel << "\t\t"
                 << current->status << "\t\t"
                 << current->timestamp << endl;
            current = current->next;
        }
    }
    
    ~EmergencyPriorityQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }
};

// Emergency Request Manager class
class EmergencyRequestManager {
private:
    EmergencyPriorityQueue pendingQueue;     // Priority Queue for PENDING requests
    int requestCounter;
    
public:
    EmergencyRequestManager() {
        requestCounter = 204; // Starting from 204 (will increment for new requests)
    }
    
    // Load emergency requests from JSON data (only pending ones)
    void loadFromJSON(const json& data) {
        // Find highest ID number to set counter
        int maxId = 0;
        for (const auto& r : data["emergencyRequests"]) {
            int id = r["id"];
            if (id > maxId) {
                maxId = id;
            }
        }
        requestCounter = maxId;
        
        // Load pending requests into priority queue
        for (const auto& r : data["emergencyRequests"]) {
            if (r["status"] == "PENDING") {
                EmergencyRequest request(r["id"], r["location"], r["requestType"], 
                                       r["urgencyLevel"], r["status"], r["timestamp"]);
                
                // Convert JSON arrays to vectors
                if (r.contains("Volunteer_Id") && r["Volunteer_Id"].is_array()) {
                    for (const auto& vid : r["Volunteer_Id"]) {
                        request.Volunteer_Id.push_back(vid);
                    }
                }
                if (r.contains("Supply_Id") && r["Supply_Id"].is_array()) {
                    for (const auto& sid : r["Supply_Id"]) {
                        request.Supply_Id.push_back(sid);
                    }
                }
                if (r.contains("vehicle_Id") && r["vehicle_Id"].is_array()) {
                    for (const auto& vid : r["vehicle_Id"]) {
                        request.vehicle_Id.push_back(vid);
                    }
                }
                
                pendingQueue.enqueue(request);
            }
        }
    }
    
    // Log new emergency request
    void logEmergencyRequest(json& data) {
        int typeChoice, urgencyChoice, locationChoice;
        string requestType, location;
        int urgencyLevel;
        
        cout << "\n=== LOG EMERGENCY REQUEST ===" << endl;
        
        // Emergency Type selection
        cout << "Select Emergency Type:" << endl;
        cout << "1. Medical" << endl;
        cout << "2. Earthquake" << endl;
        cout << "3. Search & Rescue" << endl;
        cout << "4. Flood" << endl;
        cout << "Enter choice: ";
        cin >> typeChoice;
        
        switch (typeChoice) {
            case 1: requestType = "Medical"; break;
            case 2: requestType = "Earthquake"; break;
            case 3: requestType = "Search & Rescue"; break;
            case 4: requestType = "Flood"; break;
            default:
                cout << "\n Invalid emergency type! Request cancelled." << endl;
                return;
        }
        
        // Urgency Level selection
        cout << "Select Urgency Level:" << endl;
        cout << "1. Low Priority (1)" << endl;
        cout << "2. Medium Priority (3)" << endl;
        cout << "3. High Priority (5)" << endl;
        cout << "Enter choice: ";
        cin >> urgencyChoice;
        
        switch (urgencyChoice) {
            case 1: urgencyLevel = 1; break;
            case 2: urgencyLevel = 3; break;
            case 3: urgencyLevel = 5; break;
            default:
                cout << "\n Invalid urgency level! Request cancelled." << endl;
                return;
        }
        
        // Location selection
        cout << "Select Location:" << endl;
        cout << "1. Johor" << endl;
        cout << "2. Penang" << endl;
        cout << "3. KL" << endl;
        cout << "4. Perak" << endl;
        cout << "Enter choice: ";
        cin >> locationChoice;
        
        switch (locationChoice) {
            case 1: location = "Johor"; break;
            case 2: location = "Penang"; break;
            case 3: location = "KL"; break;
            case 4: location = "Perak"; break;
            default:
                cout << "\n Invalid location! Request cancelled." << endl;
                return;
        }
        
        requestCounter++;
        EmergencyRequest newRequest(requestCounter, location, requestType, urgencyLevel);
        
        // Add to priority queue
        pendingQueue.enqueue(newRequest);
        
        // Add to JSON data
        json newRequestJson = {
            {"id", requestCounter},
            {"location", location},
            {"requestType", requestType},
            {"urgencyLevel", urgencyLevel},
            {"status", "PENDING"},
            {"Volunteer_Id", json::array()},
            {"Supply_Id", json::array()},
            {"vehicle_Id", json::array()},
            {"timestamp", newRequest.timestamp}
        };
        data["emergencyRequests"].push_back(newRequestJson);
        
        cout << "\n✓ Emergency request logged successfully!" << endl;
        cout << "Request ID: " << requestCounter << endl;
        cout << "Type: " << requestType << endl;
        cout << "Location: " << location << endl;
        cout << "Urgency Level: " << urgencyLevel << endl;
        cout << "Status: PENDING" << endl;
        cout << "Timestamp: " << newRequest.timestamp << endl;
    }
    
    // Process most critical request (change from PENDING to READY_FOR_ASSIGNMENT)
    void processMostCriticalRequest(json& data) {
        cout << "\n=== PROCESS MOST CRITICAL REQUEST ===" << endl;
        
        if (pendingQueue.isEmpty()) {
            cout << "\n No pending emergency requests to process!" << endl;
            return;
        }
        
        // Get the highest priority request
        EmergencyRequest criticalRequest = pendingQueue.dequeue();
        
        // Update status in JSON data
        for (auto& request : data["emergencyRequests"]) {
            if (request["id"] == criticalRequest.id) {
                request["status"] = "READY_FOR_ASSIGNMENT";
                break;
            }
        }
        
        cout << "\n✓ Most critical request processed successfully!" << endl;
        cout << "Request ID: " << criticalRequest.id << endl;
        cout << "Type: " << criticalRequest.requestType << endl;
        cout << "Location: " << criticalRequest.location << endl;
        cout << "Urgency Level: " << criticalRequest.urgencyLevel << endl;
        cout << "Previous Status: PENDING" << endl;
        cout << "New Status: READY_FOR_ASSIGNMENT" << endl;
        cout << "Timestamp: " << criticalRequest.timestamp << endl;
    }
    
    // View all pending requests
    void viewPendingRequests() {
        cout << "\n=== PENDING EMERGENCY REQUESTS ===" << endl;
        cout << "ID\t\tLocation\tType\t\t\tUrgency\tStatus\t\tTimestamp" << endl;
        cout << "===================================================================================" << endl;
        
        if (pendingQueue.isEmpty()) {
            cout << "No pending emergency requests." << endl;
        } else {
            pendingQueue.display();
        }
        
        cout << "\nTotal Pending Requests: " << pendingQueue.count() << endl;
        cout << "\nNote: Requests are sorted by priority (Urgency Level DESC, then FIFO)" << endl;
    }
};

// Volunteer structure
struct Volunteer {
    string id;
    string name;
    string contact;
    string skill;
    string status;
    string registeredAt;
    Volunteer* next; // For linked list implementation (acting as queue)
    
    Volunteer(string volId, string volName, string volContact, string volSkill, string volStatus = "AVAILABLE", string registered = "") {
        id = volId;
        name = volName;
        contact = volContact;
        skill = volSkill;
        status = volStatus;
        registeredAt = registered.empty() ? getCurrentTimestamp() : registered;
        next = nullptr;
    }
    
    // Function to get current timestamp
    static string getCurrentTimestamp() {
        time_t now = time(0);
        tm* timeinfo = localtime(&now);
        
        stringstream ss;
        ss << (timeinfo->tm_year + 1900) << "-"
           << setfill('0') << setw(2) << (timeinfo->tm_mon + 1) << "-"
           << setfill('0') << setw(2) << timeinfo->tm_mday << "T"
           << setfill('0') << setw(2) << timeinfo->tm_hour << ":"
           << setfill('0') << setw(2) << timeinfo->tm_min << ":"
           << setfill('0') << setw(2) << timeinfo->tm_sec << "Z";
        
        return ss.str();
    }
};

// Volunteer Queue class for FIFO operations
class VolunteerQueue {
private:
    Volunteer* front;
    Volunteer* rear;
    
public:
    VolunteerQueue() : front(nullptr), rear(nullptr) {}
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    void enqueue(const Volunteer& volunteer) {
        Volunteer* newVolunteer = new Volunteer(volunteer.id, volunteer.name, volunteer.contact, 
                                               volunteer.skill, volunteer.status, volunteer.registeredAt);
        if (isEmpty()) {
            front = rear = newVolunteer;
        } else {
            rear->next = newVolunteer;
            rear = newVolunteer;
        }
    }
    
    Volunteer dequeue() {
        if (isEmpty()) {
            throw runtime_error("Queue is empty");
        }
        Volunteer* temp = front;
        Volunteer volunteer = *temp;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp;
        return volunteer;
    }
    
    int count() {
        int cnt = 0;
        Volunteer* current = front;
        while (current) {
            cnt++;
            current = current->next;
        }
        return cnt;
    }
    
    void display() {
        Volunteer* current = front;
        while (current) {
            cout << current->id << "\t\t" 
                 << current->name << "\t\t" 
                 << current->skill << "\t\t" 
                 << current->contact << "\t\t"
                 << current->status << endl;
            current = current->next;
        }
    }
    
    ~VolunteerQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }
};

// Volunteer Manager class
class VolunteerManager {
private:
    VolunteerQueue medicalQueue;     // FIFO for Medical volunteers
    VolunteerQueue generalQueue;     // FIFO for General volunteers
    int volunteerCounter;
    
public:
    VolunteerManager() {
        volunteerCounter = 124; // Starting from V124 (will increment for new volunteers)
    }
    
    // Load volunteers from JSON data (only available ones)
    void loadFromJSON(const json& data) {
        // Find highest ID number to set counter
        int maxId = 0;
        for (const auto& v : data["volunteers"]) {
            string id = v["id"];
            int idNum = stoi(id.substr(1)); // Remove 'V' and convert to int
            if (idNum > maxId) {
                maxId = idNum;
            }
        }
        volunteerCounter = maxId;
        
        // Load available volunteers into appropriate queues
        for (const auto& v : data["volunteers"]) {
            if (v["status"] == "AVAILABLE") {
                Volunteer volunteer(v["id"], v["name"], v["contact"], v["skill"], v["status"], v["registeredAt"]);
                
                if (v["skill"] == "Medical") {
                    medicalQueue.enqueue(volunteer);
                } else if (v["skill"] == "General") {
                    generalQueue.enqueue(volunteer);
                }
            }
        }
    }
    
    // Register new volunteer
    void registerVolunteer(json& data) {
        string name, contact, skill;
        int skillChoice;
        
        cout << "\n=== REGISTER VOLUNTEER ===" << endl;
        
        cin.ignore(); // Clear input buffer
        cout << "Enter Volunteer Name: ";
        getline(cin, name);
        
        cout << "Enter Contact Info: ";
        getline(cin, contact);
        
        cout << "Select Skill Type:" << endl;
        cout << "1. Medical" << endl;
        cout << "2. General" << endl;
        cout << "Enter choice: ";
        cin >> skillChoice;
        
        switch (skillChoice) {
            case 1:
                skill = "Medical";
                break;
            case 2:
                skill = "General";
                break;
            default:
                cout << "\n Invalid skill type! Registration cancelled." << endl;
                return;
        }
        
        volunteerCounter++;
        string newId = "V" + to_string(volunteerCounter);
        
        Volunteer newVolunteer(newId, name, contact, skill);
        
        // Add to appropriate queue
        if (skill == "Medical") {
            medicalQueue.enqueue(newVolunteer);
        } else {
            generalQueue.enqueue(newVolunteer);
        }
        
        // Add to JSON data
        json newVolunteerJson = {
            {"id", newId},
            {"name", name},
            {"contact", contact},
            {"skill", skill},
            {"status", "AVAILABLE"},
            {"registeredAt", newVolunteer.registeredAt}
        };
        data["volunteers"].push_back(newVolunteerJson);
        
        cout << "\n✓ Volunteer registered successfully!" << endl;
        cout << "Volunteer ID: " << newId << endl;
        cout << "Name: " << name << endl;
        cout << "Skill: " << skill << endl;
        cout << "Contact: " << contact << endl;
        cout << "Status: AVAILABLE" << endl;
        cout << "Registered At: " << newVolunteer.registeredAt << endl;
    }
    
    // Get volunteer requirements for emergency type
    pair<int, int> getVolunteerRequirements(const string& emergencyType) {
        // Returns {medical_count, general_count}
        if (emergencyType == "Medical") {
            return {3, 0};
        } else if (emergencyType == "Earthquake") {
            return {2, 2};
        } else if (emergencyType == "Search & Rescue") {
            return {1, 6};
        } else if (emergencyType == "Flood") {
            return {2, 4};
        }
        return {0, 0};
    }
    
    // Check if we have enough volunteers for emergency type
    bool checkVolunteerAvailability(const string& emergencyType) {
        auto requirements = getVolunteerRequirements(emergencyType);
        return medicalQueue.count() >= requirements.first && 
               generalQueue.count() >= requirements.second;
    }
    
    // Get highest priority emergency request that needs volunteers (and doesn't already have volunteers)
    json getHighestPriorityRequest(const json& data) {
        json highestRequest = nullptr;
        int highestUrgency = 0;
        string earliestTime = "";
        
        for (const auto& request : data["emergencyRequests"]) {
            if (request["status"] == "READY_FOR_ASSIGNMENT" && request["Volunteer_Id"].empty()) {
                int urgency = request["urgencyLevel"];
                string timestamp = request["timestamp"];
                
                if (urgency > highestUrgency || 
                    (urgency == highestUrgency && (earliestTime.empty() || timestamp < earliestTime))) {
                    highestRequest = request;
                    highestUrgency = urgency;
                    earliestTime = timestamp;
                }
            }
        }
        
        return highestRequest;
    }
    
    // Deploy volunteers for specific emergency type
    vector<string> deployVolunteers(const string& emergencyType, json& data) {
        vector<string> deployedIds;
        auto requirements = getVolunteerRequirements(emergencyType);
        int medicalNeeded = requirements.first;
        int generalNeeded = requirements.second;
        
        try {
            // Deploy medical volunteers
            for (int i = 0; i < medicalNeeded; i++) {
                Volunteer medical = medicalQueue.dequeue();
                deployedIds.push_back(medical.id);
                markAsDeployed(medical.id, data);
            }
            
            // Deploy general volunteers
            for (int i = 0; i < generalNeeded; i++) {
                Volunteer general = generalQueue.dequeue();
                deployedIds.push_back(general.id);
                markAsDeployed(general.id, data);
            }
        } catch (const runtime_error& e) {
            cout << "\n Error during deployment: " << e.what() << endl;
        }
        
        return deployedIds;
    }
    
    // Mark volunteer as deployed in JSON
    void markAsDeployed(const string& volunteerId, json& data) {
        for (auto& v : data["volunteers"]) {
            if (v["id"] == volunteerId) {
                v["status"] = "DEPLOYED";
                break;
            }
        }
    }
    
    // Deploy volunteers to highest priority emergency
    void deployVolunteersToField(json& data) {
        cout << "\n=== DEPLOY VOLUNTEERS TO FIELD ===" << endl;
        
        // Get highest priority emergency
        json priorityRequest = getHighestPriorityRequest(data);
        
        if (priorityRequest == nullptr) {
            cout << "\n No emergency requests available for assignment!" << endl;
            return;
        }
        
        string emergencyType = priorityRequest["requestType"];
        string location = priorityRequest["location"];
        int urgency = priorityRequest["urgencyLevel"];
        int requestId = priorityRequest["id"];
        
        cout << "\nHighest Priority Emergency:" << endl;
        cout << "Request ID: " << requestId << endl;
        cout << "Location: " << location << endl;
        cout << "Type: " << emergencyType << endl;
        cout << "Urgency Level: " << urgency << endl;
        
        // Check volunteer availability
        if (!checkVolunteerAvailability(emergencyType)) {
            auto requirements = getVolunteerRequirements(emergencyType);
            cout << "\n Not enough volunteers available for this emergency type!" << endl;
            cout << "Required volunteers for " << emergencyType << ":" << endl;
            cout << "- Medical Volunteers: " << requirements.first << " (Available: " << medicalQueue.count() << ")" << endl;
            cout << "- General Volunteers: " << requirements.second << " (Available: " << generalQueue.count() << ")" << endl;
            return;
        }
        
        // Deploy volunteers
        vector<string> deployedIds = deployVolunteers(emergencyType, data);
        
        // Update emergency request with volunteer IDs (DON'T change status yet)
        for (auto& request : data["emergencyRequests"]) {
            if (request["id"] == requestId) {
                request["Volunteer_Id"] = deployedIds;
                
                // Only change status to FULLY_DISPATCHED if ALL requirements are met
                bool hasSupplies = !request["Supply_Id"].empty();
                bool hasVolunteers = !request["Volunteer_Id"].empty();
                bool hasVehicles = !request["vehicle_Id"].empty();
                
                if (hasSupplies && hasVolunteers && hasVehicles) {
                    request["status"] = "FULLY_DISPATCHED";
                }
                // Don't change status if not all requirements are met - keep READY_FOR_ASSIGNMENT
                break;
            }
        }
        
        cout << "\n✓ Volunteers successfully deployed!" << endl;
        cout << "Deployed " << deployedIds.size() << " volunteers to " << location << endl;
        cout << "Volunteer IDs: ";
        for (size_t i = 0; i < deployedIds.size(); i++) {
            cout << deployedIds[i];
            if (i < deployedIds.size() - 1) cout << ", ";
        }
        cout << endl;
    }
    
    // View all available volunteers
    void viewVolunteers() {
        cout << "\n=== AVAILABLE VOLUNTEERS ===" << endl;
        cout << "ID\t\tName\t\t\tSkill\t\tContact\t\t\tStatus" << endl;
        cout << "===============================================================================" << endl;
        
        cout << "\n--- MEDICAL VOLUNTEERS (Queue - FIFO) ---" << endl;
        if (medicalQueue.isEmpty()) {
            cout << "No Medical volunteers available." << endl;
        } else {
            medicalQueue.display();
        }
        
        cout << "\n--- GENERAL VOLUNTEERS (Queue - FIFO) ---" << endl;
        if (generalQueue.isEmpty()) {
            cout << "No General volunteers available." << endl;
        } else {
            generalQueue.display();
        }
        
        cout << "\nSummary:" << endl;
        cout << "Medical Volunteers: " << medicalQueue.count() << " available" << endl;
        cout << "General Volunteers: " << generalQueue.count() << " available" << endl;
    }
};

// Supply structure to hold supply data
struct Supply {
    string id;
    string type;
    bool is_deployed;
    string packedAt;
    Supply* next; // For linked list implementation (acting as stack)
    
    Supply(string supplyId, string supplyType, bool deployed = false, string packed = "") {
        id = supplyId;
        type = supplyType;
        is_deployed = deployed;
        packedAt = packed.empty() ? getCurrentTimestamp() : packed;
        next = nullptr;
    }
    
    // Function to get current timestamp
    static string getCurrentTimestamp() {
        time_t now = time(0);
        tm* timeinfo = localtime(&now);
        
        stringstream ss;
        ss << (timeinfo->tm_year + 1900) << "-"
           << setfill('0') << setw(2) << (timeinfo->tm_mon + 1) << "-"
           << setfill('0') << setw(2) << timeinfo->tm_mday << "T"
           << setfill('0') << setw(2) << timeinfo->tm_hour << ":"
           << setfill('0') << setw(2) << timeinfo->tm_min << ":"
           << setfill('0') << setw(2) << timeinfo->tm_sec << "Z";
        
        return ss.str();
    }
};

// Supply Node for Queue implementation
struct SupplyNode {
    Supply data;
    SupplyNode* next;
    
    SupplyNode(const Supply& supply) : data(supply), next(nullptr) {}
};

// Supply Queue class for FIFO operations (perishable items)
class SupplyQueue {
private:
    SupplyNode* front;
    SupplyNode* rear;
    
public:
    SupplyQueue() : front(nullptr), rear(nullptr) {}
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    void enqueue(const Supply& supply) {
        SupplyNode* newNode = new SupplyNode(supply);
        if (isEmpty()) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
    }
    //algo for supply
    Supply dequeue() {
        if (isEmpty()) {
            throw runtime_error("Queue is empty");
        }
        SupplyNode* temp = front;
        Supply supply = temp->data;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp;
        return supply;
    }
    
    int count() {
        int cnt = 0;
        SupplyNode* current = front;
        while (current) {
            cnt++;
            current = current->next;
        }
        return cnt;
    }
    
    void display() {
        SupplyNode* current = front;
        while (current) {
            cout << current->data.id << "\t\t" 
                 << current->data.type << "\t\t" 
                 << current->data.packedAt << "\t" 
                 << "READY" << endl;
            current = current->next;
        }
    }
    
    ~SupplyQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }
};

// Supply Stack class for LIFO operations (durable items)
class SupplyStack {
private:
    Supply* top;
    
public:
    SupplyStack() {
        top = nullptr;
    }
    
    bool isEmpty() {
        return top == nullptr;
    }
    
    void push(const Supply& supply) {
        Supply* newSupply = new Supply(supply.id, supply.type, supply.is_deployed, supply.packedAt);
        newSupply->next = top;
        top = newSupply;
    }
    
    Supply pop() {
        if (isEmpty()) {
            throw runtime_error("Stack is empty");
        }
        Supply* temp = top;
        Supply supply = *temp;
        top = top->next;
        delete temp;
        return supply;
    }
    
    int count() {
        int cnt = 0;
        Supply* current = top;
        while (current) {
            cnt++;
            current = current->next;
        }
        return cnt;
    }
    
    void display() {
        Supply* current = top;
        while (current) {
            cout << current->id << "\t\t" 
                 << current->type << "\t\t" 
                 << current->packedAt << "\t" 
                 << "READY" << endl;
            current = current->next;
        }
    }
    
    ~SupplyStack() {
        while (top != nullptr) {
            Supply* temp = top;
            top = top->next;
            delete temp;
        }
    }
};

// Supply Manager class that handles all supply operations
class SupplyManager {
private:
    SupplyQueue foodQueue;           // FIFO for Food & Drink
    SupplyQueue medicalQueue;        // FIFO for Medical Supplies
    SupplyStack shelterStack;        // LIFO for Shelter Materials
    SupplyStack rescueStack;         // LIFO for Rescue Equipment
    int supplyCounter;
    
public:
    SupplyManager() {
        supplyCounter = 125; // Starting from S125 and will increment for new supplies
    }
    
    // Load supplies from JSON data (only non-deployed ones)
    void loadFromJSON(const json& data) {
        // Find highest ID number to set counter
        int maxId = 0;
        for (const auto& s : data["supplies"]) {
            string id = s["id"];
            int idNum = stoi(id.substr(1)); // Remove 'S' and convert to int
            if (idNum > maxId) {
                maxId = idNum;
            }
        }
        supplyCounter = maxId;
        
        // Load non-deployed supplies into appropriate containers
        for (const auto& s : data["supplies"]) {
            if (!s["is_deployed"]) {
                Supply supply(s["id"], s["type"], s["is_deployed"], s["packedAt"]);
                
                if (s["type"] == "Food & Drink") {
                    foodQueue.enqueue(supply);
                } else if (s["type"] == "Medical Supplies") {
                    medicalQueue.enqueue(supply);
                } else if (s["type"] == "Shelter Materials") {
                    shelterStack.push(supply);
                } else if (s["type"] == "Rescue Equipment") {
                    rescueStack.push(supply);
                }
            }
        }
    }
    
    // Add new supply (based on type)
    void addSupply(string type, json& data) {
        supplyCounter++;
        string newId = "S" + to_string(supplyCounter);
        
        Supply newSupply(newId, type);
        
        // Add to appropriate container
        if (type == "Food & Drink") {
            foodQueue.enqueue(newSupply);
        } else if (type == "Medical Supplies") {
            medicalQueue.enqueue(newSupply);
        } else if (type == "Shelter Materials") {
            shelterStack.push(newSupply);
        } else if (type == "Rescue Equipment") {
            rescueStack.push(newSupply);
        }
        
        // Add to JSON data
        json newSupplyJson = {
            {"id", newId},
            {"type", type},
            {"is_deployed", false},
            {"packedAt", newSupply.packedAt}
        };
        data["supplies"].push_back(newSupplyJson);
        
        cout << "\n✓ Supply added successfully!" << endl;
        cout << "Supply ID: " << newId << endl;
        cout << "Type: " << type << endl;
        cout << "Packed At: " << newSupply.packedAt << endl;
        cout << "Status: Ready for dispatch" << endl;
    }
    //algo for request  
    // Get highest priority emergency request that needs supplies (and doesn't already have supplies)
    json getHighestPriorityRequest(const json& data) {
        json highestRequest = nullptr;
        int highestUrgency = 0;
        string earliestTime = "";
        
        for (const auto& request : data["emergencyRequests"]) {
            if (request["status"] == "READY_FOR_ASSIGNMENT" && request["Supply_Id"].empty()) {
                int urgency = request["urgencyLevel"];
                string timestamp = request["timestamp"];
                
                if (urgency > highestUrgency || 
                    (urgency == highestUrgency && (earliestTime.empty() || timestamp < earliestTime))) {
                    highestRequest = request;
                    highestUrgency = urgency;
                    earliestTime = timestamp;
                }
            }
        }
        
        return highestRequest;
    }
    
    // Check if we have enough supplies for emergency type
    bool checkSupplyAvailability(const string& emergencyType) {
        if (emergencyType == "Medical") {
            return medicalQueue.count() >= 3;
        } else if (emergencyType == "Earthquake") {
            return medicalQueue.count() >= 2 && 
                   shelterStack.count() >= 2 && 
                   rescueStack.count() >= 2;
        } else if (emergencyType == "Search & Rescue") {
            return foodQueue.count() >= 2 && 
                   shelterStack.count() >= 1 && 
                   rescueStack.count() >= 1;
        } else if (emergencyType == "Flood") {
            return medicalQueue.count() >= 2 && 
                   shelterStack.count() >= 2 && 
                   rescueStack.count() >= 2;
        }
        return false;
    }
    
    // Dispatch supplies for specific emergency type
    vector<string> dispatchSupplies(const string& emergencyType, json& data) {
        vector<string> dispatchedIds;
        
        try {
            if (emergencyType == "Medical") {
                // 3x Medical Supplies
                for (int i = 0; i < 3; i++) {
                    Supply medical = medicalQueue.dequeue();
                    dispatchedIds.push_back(medical.id);
                    markAsDeployed(medical.id, data);
                }
            } else if (emergencyType == "Earthquake") {
                // 2x Medical + 2x Shelter + 2x Rescue Equipment
                for (int i = 0; i < 2; i++) {
                    Supply medical = medicalQueue.dequeue();
                    Supply shelter = shelterStack.pop();
                    Supply rescue = rescueStack.pop();
                    
                    dispatchedIds.push_back(medical.id);
                    dispatchedIds.push_back(shelter.id);
                    dispatchedIds.push_back(rescue.id);
                    
                    markAsDeployed(medical.id, data);
                    markAsDeployed(shelter.id, data);
                    markAsDeployed(rescue.id, data);
                }
            } else if (emergencyType == "Search & Rescue") {
                // 2x Food & Drink + 1x Shelter + 1x Rescue Equipment
                for (int i = 0; i < 2; i++) {
                    Supply food = foodQueue.dequeue();
                    dispatchedIds.push_back(food.id);
                    markAsDeployed(food.id, data);
                }
                Supply shelter = shelterStack.pop();
                Supply rescue = rescueStack.pop();
                dispatchedIds.push_back(shelter.id);
                dispatchedIds.push_back(rescue.id);
                markAsDeployed(shelter.id, data);
                markAsDeployed(rescue.id, data);
            } else if (emergencyType == "Flood") {
                // 2x Medical + 2x Shelter + 2x Rescue Equipment
                for (int i = 0; i < 2; i++) {
                    Supply medical = medicalQueue.dequeue();
                    Supply shelter = shelterStack.pop();
                    Supply rescue = rescueStack.pop();
                    
                    dispatchedIds.push_back(medical.id);
                    dispatchedIds.push_back(shelter.id);
                    dispatchedIds.push_back(rescue.id);
                    
                    markAsDeployed(medical.id, data);
                    markAsDeployed(shelter.id, data);
                    markAsDeployed(rescue.id, data);
                }
            }
        } catch (const runtime_error& e) {
            cout << "\n Error during dispatch: " << e.what() << endl;
        }
        
        return dispatchedIds;
    }
    
    // Mark supply as deployed in JSON
    void markAsDeployed(const string& supplyId, json& data) {
        for (auto& s : data["supplies"]) {
            if (s["id"] == supplyId) {
                s["is_deployed"] = true;
                break;
            }
        }
    }
    
    // Send supply boxes to highest priority emergency
    void sendSupplyBoxes(json& data) {
        cout << "\n=== SEND SUPPLY BOXES ===" << endl;
        cout << "Press 1 to confirm dispatch (or any other key to cancel): ";
        
        int confirmation;
        cin >> confirmation;
        
        if (confirmation != 1) {
            cout << "\n Dispatch cancelled." << endl;
            return;
        }
        
        // Get highest priority emergency
        json priorityRequest = getHighestPriorityRequest(data);
        
        if (priorityRequest == nullptr) {
            cout << "\n No emergency requests available for assignment!" << endl;
            return;
        }
        
        string emergencyType = priorityRequest["requestType"];
        string location = priorityRequest["location"];
        int urgency = priorityRequest["urgencyLevel"];
        int requestId = priorityRequest["id"];
        
        cout << "\nHighest Priority Emergency:" << endl;
        cout << "Request ID: " << requestId << endl;
        cout << "Location: " << location << endl;
        cout << "Type: " << emergencyType << endl;
        cout << "Urgency Level: " << urgency << endl;
        
        // Check supply availability
        if (!checkSupplyAvailability(emergencyType)) {
            cout << "\n Not enough supplies available for this emergency type!" << endl;
            cout << "Required supplies for " << emergencyType << ":" << endl;
            
            if (emergencyType == "Medical") {
                cout << "- Medical Supplies: 3 (Available: " << medicalQueue.count() << ")" << endl;
            } else if (emergencyType == "Earthquake") {
                cout << "- Medical Supplies: 2 (Available: " << medicalQueue.count() << ")" << endl;
                cout << "- Shelter Materials: 2 (Available: " << shelterStack.count() << ")" << endl;
                cout << "- Rescue Equipment: 2 (Available: " << rescueStack.count() << ")" << endl;
            } else if (emergencyType == "Search & Rescue") {
                cout << "- Food & Drink: 2 (Available: " << foodQueue.count() << ")" << endl;
                cout << "- Shelter Materials: 1 (Available: " << shelterStack.count() << ")" << endl;
                cout << "- Rescue Equipment: 1 (Available: " << rescueStack.count() << ")" << endl;
            } else if (emergencyType == "Flood") {
                cout << "- Medical Supplies: 2 (Available: " << medicalQueue.count() << ")" << endl;
                cout << "- Shelter Materials: 2 (Available: " << shelterStack.count() << ")" << endl;
                cout << "- Rescue Equipment: 2 (Available: " << rescueStack.count() << ")" << endl;
            }
            return;
        }
        
        // Dispatch supplies
        vector<string> dispatchedIds = dispatchSupplies(emergencyType, data);
        
        // Update emergency request with supply IDs
        for (auto& request : data["emergencyRequests"]) {
            if (request["id"] == requestId) {
                request["Supply_Id"] = dispatchedIds;
                
                // Only change status to FULLY_DISPATCHED if ALL requirements are met
                bool hasSupplies = !request["Supply_Id"].empty();
                bool hasVolunteers = !request["Volunteer_Id"].empty();
                bool hasVehicles = !request["vehicle_Id"].empty();
                
                if (hasSupplies && hasVolunteers && hasVehicles) {
                    request["status"] = "FULLY_DISPATCHED";
                }
                break;
            }
        }
        
        cout << "\n✓ Supplies successfully dispatched!" << endl;
        cout << "Dispatched " << dispatchedIds.size() << " supply boxes to " << location << endl;
        cout << "Supply IDs: ";
        for (size_t i = 0; i < dispatchedIds.size(); i++) {
            cout << dispatchedIds[i];
            if (i < dispatchedIds.size() - 1) cout << ", ";
        }
        cout << endl;
    }
    
    // View all supplies waiting for dispatch
    void viewAllSupplies() {
        cout << "\n=== SUPPLIES WAITING FOR DISPATCH ===" << endl;
        cout << "ID\t\tType\t\t\tPacked At\t\tStatus" << endl;
        cout << "================================================================" << endl;
        
        cout << "\n--- FOOD & DRINK (Queue - FIFO) ---" << endl;
        if (foodQueue.isEmpty()) {
            cout << "No Food & Drink supplies available." << endl;
        } else {
            foodQueue.display();
        }
        
        cout << "\n--- MEDICAL SUPPLIES (Queue - FIFO) ---" << endl;
        if (medicalQueue.isEmpty()) {
            cout << "No Medical supplies available." << endl;
        } else {
            medicalQueue.display();
        }
        
        cout << "\n--- SHELTER MATERIALS (Stack - LIFO) ---" << endl;
        if (shelterStack.isEmpty()) {
            cout << "No Shelter Materials available." << endl;
        } else {
            shelterStack.display();
        }
        
        cout << "\n--- RESCUE EQUIPMENT (Stack - LIFO) ---" << endl;
        if (rescueStack.isEmpty()) {
            cout << "No Rescue Equipment available." << endl;
        } else {
            rescueStack.display();
        }
        
        cout << "\nSummary:" << endl;
        cout << "Food & Drink: " << foodQueue.count() << " boxes" << endl;
        cout << "Medical Supplies: " << medicalQueue.count() << " boxes" << endl;
        cout << "Shelter Materials: " << shelterStack.count() << " boxes" << endl;
        cout << "Rescue Equipment: " << rescueStack.count() << " boxes" << endl;
    }
};

// JSON Handling Functions (same as your friend's code)
json loadData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        exit(1);
    }
    json data;
    try {
        file >> data;
    }
    catch (const json::parse_error& e) {
        cerr << "JSON Parse Error: " << e.what() << endl;
        exit(1);
    }
    return data;
}

void saveData(const json& data, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to write file: " << filename << endl;
        exit(1);
    }
    file << data.dump(4);
}

// Global variables
SupplyManager supplyManager;
VolunteerManager volunteerManager;
EmergencyRequestManager emergencyManager;
TransportUnitManager transportManager;
const string FILENAME = "Data.TXT";
json jsonData;

// Function prototypes
void displayMainMenu();
void displaySupplyBaseMenu();
void displayVolunteerMenu();
void displayEmergencyMenu();
void displayTransportMenu();
void handleSupplyBaseManager();
void handleVolunteerOperations();
void handleEmergencyCoordinator();
void handleTransportScheduler();
void addSupplyMenu();
void clearScreen();

int main() {
    // Load data from JSON file at startup
    jsonData = loadData(FILENAME);
    supplyManager.loadFromJSON(jsonData);
    volunteerManager.loadFromJSON(jsonData);
    emergencyManager.loadFromJSON(jsonData);
    transportManager.loadFromJSON(jsonData);
    
    int choice;
    
    do {
        displayMainMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                handleSupplyBaseManager();
                break;
            case 2:
                handleVolunteerOperations();
                break;
            case 3:
                handleEmergencyCoordinator();
                break;
            case 4:
                handleTransportScheduler();
                break;
            case 0:
                cout << "\n👋 Thank you for using the Emergency Supply Management System!" << endl;
                break;
            default:
                cout << "\n Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
        }
    } while (choice != 0);
    
    return 0;
}

void displayMainMenu() {
    clearScreen();
    cout << "=== EMERGENCY SUPPLY MANAGEMENT SYSTEM ===" << endl;
    cout << "1. SUPPLY BASE MANAGER" << endl;
    cout << "2. VOLUNTEER OPERATIONS OFFICER" << endl;
    cout << "3. EMERGENCY REQUEST COORDINATOR" << endl;
    cout << "4. TRANSPORT UNIT SCHEDULER" << endl;
    cout << "0. Exit" << endl;
    cout << "==========================================" << endl;
}

void displayTransportMenu() {
    clearScreen();
    cout << "=== TRANSPORT UNIT SCHEDULER MENU ===" << endl;
    cout << "1. Add Vehicle" << endl;
    cout << "2. Assign Vehicle to Emergency" << endl;
    cout << "3. Return Vehicle from Mission" << endl;
    cout << "4. Display Vehicle Schedule" << endl;
    cout << "0. Return to Main Menu" << endl;
    cout << "=====================================" << endl;
}

void handleTransportScheduler() {
    int choice;
    
    do {
        displayTransportMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                transportManager.addVehicle(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 2:
                transportManager.assignVehicleToEmergency(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 3:
                transportManager.returnVehicleFromMission(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 4:
                transportManager.displayVehicleSchedule();
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 0:
                cout << "\n Returning to Main Menu..." << endl;
                break;
            default:
                cout << "\n Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
        }
    } while (choice != 0);
}

void displayEmergencyMenu() {
    clearScreen();
    cout << "=== EMERGENCY REQUEST COORDINATOR MENU ===" << endl;
    cout << "1. Log Emergency Request" << endl;
    cout << "2. Process Most Critical Request" << endl;
    cout << "3. View Pending Requests" << endl;
    cout << "0. Return to Main Menu" << endl;
    cout << "==========================================" << endl;
}

void handleEmergencyCoordinator() {
    int choice;
    
    do {
        displayEmergencyMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                emergencyManager.logEmergencyRequest(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 2:
                emergencyManager.processMostCriticalRequest(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 3:
                emergencyManager.viewPendingRequests();
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 0:
                cout << "\n Returning to Main Menu..." << endl;
                break;
            default:
                cout << "\n Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
        }
    } while (choice != 0);
}

void displayVolunteerMenu() {
    clearScreen();
    cout << "=== VOLUNTEER OPERATIONS OFFICER MENU ===" << endl;
    cout << "1. Register Volunteer" << endl;
    cout << "2. Deploy Volunteers" << endl;
    cout << "3. View Volunteers" << endl;
    cout << "0. Return to Main Menu" << endl;
    cout << "=========================================" << endl;
}

void handleVolunteerOperations() {
    int choice;
    
    do {
        displayVolunteerMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                volunteerManager.registerVolunteer(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 2:
                volunteerManager.deployVolunteersToField(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 3:
                volunteerManager.viewVolunteers();
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 0:
                cout << "\n Returning to Main Menu..." << endl;
                break;
            default:
                cout << "\n Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
        }
    } while (choice != 0);
}

void displaySupplyBaseMenu() {
    clearScreen();
    cout << "=== SUPPLY BASE MANAGER MENU ===" << endl;
    cout << "1. Add Supply" << endl;
    cout << "2. Send Boxes" << endl;
    cout << "3. View All Supplies" << endl;
    cout << "0. Return to Main Menu" << endl;
    cout << "================================" << endl;
}

void handleSupplyBaseManager() {
    int choice;
    
    do {
        displaySupplyBaseMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                addSupplyMenu();
                break;
            case 2:
                supplyManager.sendSupplyBoxes(jsonData);
                saveData(jsonData, FILENAME); // Save changes to file
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 3:
                supplyManager.viewAllSupplies();
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 0:
                cout << "\n Returning to Main Menu..." << endl;
                break;
            default:
                cout << "\n Invalid choice! Please try again." << endl;
                cout << "Press Enter to continue...";
                cin.ignore();
                cin.get();
        }
    } while (choice != 0);
}

void addSupplyMenu() {
    clearScreen();
    cout << "=== ADD SUPPLY ===" << endl;
    cout << "What type of supply:" << endl;
    cout << "1. Food & Drink" << endl;
    cout << "2. Medical Supplies" << endl;
    cout << "3. Shelter Materials" << endl;
    cout << "4. Rescue Equipment" << endl;
    cout << "0. Cancel" << endl;
    cout << "==================" << endl;
    
    int typeChoice;
    cout << "Enter your choice: ";
    cin >> typeChoice;
    
    string supplyType;
    switch (typeChoice) {
        case 1:
            supplyType = "Food & Drink";
            break;
        case 2:
            supplyType = "Medical Supplies";
            break;
        case 3:
            supplyType = "Shelter Materials";
            break;
        case 4:
            supplyType = "Rescue Equipment";
            break;
        case 0:
            cout << "\nOperation cancelled." << endl;
            cout << "Press Enter to continue...";
            cin.ignore();
            cin.get();
            return;
        default:
            cout << "\nInvalid supply type! Operation cancelled." << endl;
            cout << "Press Enter to continue...";
            cin.ignore();
            cin.get();
            return;
    }
    
    supplyManager.addSupply(supplyType, jsonData);
    saveData(jsonData, FILENAME); // Save changes to file
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

void clearScreen() {
    // Clear screen for better user experience
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}