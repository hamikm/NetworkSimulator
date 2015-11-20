/*
 * See header file for additional comments.
 */

#include "netnode.h"

/*
 * Print helper function which partially overrides the one in @c netdevice.
 * @param os The output stream to which to write.
 */
void netnode::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [node. links: { ";
	for (unsigned int i = 0;
			i < (links.size() == 0 ? 0 : links.size() - 1); i++) {
		os << links[i]->getName() << ", ";
	}
	if (links.size() > 0) {
		os << links[links.size() - 1]->getName();
	}
	os << " } ]";
}
