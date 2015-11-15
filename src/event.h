/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef EVENT_H
#define EVENT_H

#include <iostream>

using namespace std;

/**
 * Parent class of events in a discrete event-driven network simulation.
 * Includes a time attribute as well as comparison operator overrides which can
 * be used as comparators in container templates.
 */
class event {

private:

	/** Time of this event. */
	double time;

	/** ID number of this object. */
	long id;

public:

	/** Unique ID number generator. Initialized in corresponding cpp file. */
	static long id_generator;

	/**
	 * Initializes this event's time to the given one and sets the event id
	 * to whatever the static ID generates spits out.
	 */
	event(double time) : time(time) {
		id = id_generator++;
	}

	virtual ~event() {}

	double getTime() const { return time; }

	long getId() const { return id; }

	/**
	 * Subclasses--i.e. more specific events--will run operations like
	 * sending packets, adding new events to the simulation event queue, and
	 * logging data in this function.
	 */
	virtual void execute() {}

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write event information.
	 */
	virtual void printHelper(std::ostream &os) const {
		os << "[event. id: " << id << ", time: " << time << "]";
	}
};


// TODO write a comparator functor


/**
 * Output operator override for printing contents of the given event
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c event object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, const event &e) {
	e.printHelper(os);
	return os;
}

#endif // EVENT_H
