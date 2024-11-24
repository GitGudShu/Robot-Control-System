# **Robot Manufacturing Control System**

## **Overview**
This project is part of the FISA INFO curriculum at UTBM, developed by Thomas Chu and Léon Lozahic. It involves creating a manufacturing control system simulation where multiple robots collaborate and execute production tasks. The project aims to explore concurrency, task management, and inter-process communication.

## **Project Goals**
The main objective is to simulate a manufacturing environment where robots perform specific tasks in coordination. The system prioritizes efficiency, resource management, and error handling.

## **Potential Features**
Depending on the time available for development, the following features may be included:
- Management of multiple robots, each performing a distinct role.
- Coordination and communication between robots to manage dependencies.
- Dynamic task assignment and prioritization.
- Handling shared resources to avoid conflicts.
- Simulation of work durations and robot downtimes.
- Fault tolerance for handling robot failures and recovery.
- Reporting of production metrics and robot performance.

## **Technologies**
- **Programming Language**: C
- **Concurrency**: Threads or processes for robot simulation.
- **Inter-Process Communication (IPC)**: To coordinate tasks and manage shared resources.

## **How to Build and Run**
1. **Requirements**:
   - A C compiler (e.g., GCC).
   - A development environment supporting C (e.g., Visual Studio Code).

2. **Compiling the Project**:
   Compile the main source file using:
   ```bash
   gcc -o robot_simulator main.c
   ```

3. **Running the Program**:
   Run the compiled binary:
   ```bash
   ./robot_simulator
   ```

## **Development Team**
- **Thomas Chu**
- **Léon Lozahic**

## **Acknowledgments**
This project is part of the coursework for the FISA INFO program at UTBM. Thanks to the faculty for providing the opportunity and resources to undertake this project.
