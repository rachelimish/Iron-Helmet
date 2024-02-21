#include "Warning.h"
#pragma once
struct Node {
    Warning data;
    Node* next;
};
class WarningQueue
{
private:
    Node* front;
    Node* rear;
public:
    WarningQueue();
    // Function to check if the queue is empty
    bool isEmpty();
    // Function to enqueue (add Warning to the beginning of the queue)
    void enqueue(Warning data);
    // Function to dequeue (remove element from the end of the queue)
    void dequeue();
    // Function to display the elements in the queue
    void displayQueue();
   // Function that kills processes that have undergone a lot of processing time and are not relevant to the location evaluation
    void KillingIrrelevantProcesses();
};

