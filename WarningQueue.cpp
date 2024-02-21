#include "WarningQueue.h"
#include <iostream>

WarningQueue::WarningQueue()
{
	this->front = nullptr;
	this->rear = nullptr;
}

bool WarningQueue::isEmpty()
{
	return (front == nullptr);
}

void WarningQueue::enqueue(Warning data)
{
    {
        Node* newNode = new Node;
        newNode->data = data;
        newNode->next = nullptr;

        if (isEmpty()) {
            front = rear = newNode;
        }
        else {
            newNode->next = front;
            front = newNode;
        }
    }
}

void WarningQueue::dequeue()
{
    {
        if (isEmpty()) {
            std::cout << "Queue is empty. Cannot dequeue." << std::endl;
        }
        else {
            Node* temp = front;
            if (front == rear) {
                front = rear = nullptr;
            }
            else {
                while (temp->next != rear) {
                    temp = temp->next;
                }
                rear = temp;
                temp = temp->next;
                rear->next = nullptr;
            }

            std::cout << temp->data.GetName() << " dequeued from the queue." << std::endl;
            delete temp;
        }
    }

}

void WarningQueue::displayQueue()
{
    {
        Node* current = front;
        if (isEmpty()) {
            std::cout << "Queue is empty." << std::endl;
        }
        else {
            std::cout << "Elements in the queue: ";
            while (current != nullptr) {
                std::cout << current->data.GetName() << " ";
                current = current->next;
            }
            std::cout << std::endl;
        }
    }

}

void WarningQueue::KillingIrrelevantProcesses()
{
    while (true)
    {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - rear->data.GetTimeStart()).count();
        if (elapsed_time >= 5) {
            //DO:need to check how to kill the thread.
            dequeue();
        }
    }

}
