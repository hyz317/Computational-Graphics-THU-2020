#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {
        // cout << "OH! some terrible things might happen." << endl;
    }

    explicit Group (int num_objects) {
        objects.resize(num_objects);
    }

    ~Group() override {

    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool crossed = false;
        for (auto i : objects) {
            // std::cout << "fuck!!!!!!!!!!!!!!!!! " << h.getT() << std::endl;
            if (i->intersect(r, h, tmin))
                crossed = true;
        }
        return crossed;
    }

    void addObject(int index, Object3D *obj) {
        objects[index] = obj;
        // std::cout << obj->material->diff_factor << ' ' << obj->material->spec_factor << ' ' << obj->material->refr_factor << std::endl;
    }

    int getGroupSize() {
        return objects.size();
    }

private:
    std::vector<Object3D*> objects;
};

#endif
	
