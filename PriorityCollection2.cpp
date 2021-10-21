#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <map>
#include <utility>
#include <vector>
#include <list>

using namespace std;

template <typename T>
class PriorityCollection {
public:
    using Id = int;
    using Priority = int;

    Id Add(T object) {
        objects.push_back(move(object));
        object_priorities.push_back(0);
        Id id = objects.size() - 1;
        priorities[{object_priorities.back(), id}] = --objects.end();
        return id;
    }

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
        IdOutputIt ids_begin) {
        for (; range_begin != range_end; ++range_begin) {
            *(ids_begin++) = Add(move(*range_begin));
        }
    }

    bool IsValid(Id id) const {
        return (id < static_cast<int>(objects.size())) && (object_priorities[id] >= 0);
    }

    const T& Get(Id id) const {
        return objects[id];
    }

    void Promote(Id id) {
        int& priority = object_priorities[id];
        priorities.erase({ priority++, id });
        priorities[{priority, id}] = objects.begin() + id;
    }

    pair<const T&, int> GetMax() const {
        pair<Priority, Id> p = (--priorities.end())->first;
        return { objects[p.second], p.first };
    }

    pair<T, int> PopMax() {
        pair<Priority, Id> p = (--priorities.end())->first;
        Id id = p.second;
        pair<T, int> result =  make_pair(move(objects[id]), p.first);
        object_priorities[id] = -1;
        priorities.erase(p);
        return result;
    }

private:
    vector<T> objects;
    vector<Priority> object_priorities;
    map<pair<Priority, Id>, typename vector<T>::iterator> priorities;
};


class StringNonCopyable : public string {
public:
    using string::string;
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    {
        ASSERT_EQUAL(strings.Get(white_id), "white");
        ASSERT(strings.IsValid(red_id));
    }

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }

    {
        ASSERT_EQUAL(strings.GetMax().first, "red");
    }

    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }

}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);
    return 0;
}