#include "../test_lib/test_lib.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/random.h"

using namespace tengu;
using namespace test_lib;

enum {
    e_type_obj_1,
    e_type_obj_2,
    e_type_obj_3,
};

struct test_obj_1_t : public object_t {
    test_obj_1_t(object_service_t)
        : object_t(type())
    {
        ref_.dec();
    }

    static object_type_t type() {
        return e_type_obj_1;
    }

    static object_factory_t::creator_t* creator() {
        return new object_create_t<test_obj_1_t>();
    }

    static uint32_t magic() {
        return 0x12345678;
    }
};

struct test_obj_2_t : public object_t {
    test_obj_2_t(object_service_t)
        : object_t(type())
    {
    }

    static object_type_t type() {
        return e_type_obj_2;
    }

    static object_factory_t::creator_t* creator() {
        return new object_create_t<test_obj_2_t>();
    }
};

struct test_obj_3_t: public object_t {
    struct make_t: public object_factory_t::creator_t {
        make_t()
            : counter_(0)
        {
        }

        virtual object_t* create(object_type_t, object_service_t service) {
            ++counter_;
            object_t* obj = new test_obj_3_t(service);
            return obj;
        }

        virtual void destroy(object_t* obj) {
            delete obj;
            --counter_;
        }
        int counter_;
    };

    test_obj_3_t(object_service_t)
        : object_t(type())
    {
        // give up our own reference
        ref_.dec();
    }

    static object_type_t type() {
        return e_type_obj_3;
    }
};

struct test_object_1_t: public test_t {

    test_object_1_t()
            : test_t("test_object_1_t") {}

    virtual bool run() override {
        using namespace tengu;

        object_factory_t factory(nullptr);

        factory.add_creator<test_obj_1_t>();
        factory.add_creator<test_obj_2_t>();

        object_ref_t r1 = factory.create<test_obj_1_t>();
        object_ref_t r2 = factory.create<test_obj_2_t>();

        object_ref_t r3 = factory.create(e_type_obj_1);
        object_ref_t r4 = factory.create(e_type_obj_2);

        TEST_ASSERT(r1.get().is_a(e_type_obj_1));
        TEST_ASSERT(!r1.get().is_a(e_type_obj_2));

        TEST_ASSERT(r2.get().is_a(e_type_obj_2));
        TEST_ASSERT(!r2.get().is_a(e_type_obj_1));

        TEST_ASSERT(r3.get().is_a(e_type_obj_1));
        TEST_ASSERT(!r3.get().is_a(e_type_obj_2));

        TEST_ASSERT(r4.get().is_a(e_type_obj_2));
        TEST_ASSERT(!r4.get().is_a(e_type_obj_1));

        TEST_ASSERT(!r1->is_a(r2.get()));
        TEST_ASSERT(r1->is_a(r3.get()));
        TEST_ASSERT(!r1->is_a(r4.get()));

        TEST_ASSERT(!r2->is_a(r1.get()));
        TEST_ASSERT(!r2->is_a(r3.get()));
        TEST_ASSERT(r2->is_a(r4.get()));

        TEST_ASSERT(r1->cast<test_obj_1_t>().magic() == test_obj_1_t::magic());

        return true;
    }
};

struct test_object_2_t: public test_t {

    test_object_2_t()
            : test_t("test_object_2_t") {}

    virtual bool run() override {
        using namespace tengu;

        object_ref_t ref1;
        TEST_ASSERT(!ref1.valid());

        test_obj_3_t::make_t *make = new test_obj_3_t::make_t;

        object_factory_t factory(nullptr);
        factory.add_creator(e_type_obj_3, make);

        TEST_ASSERT(make->counter_ == 0);
        {
            object_ref_t obj1 = factory.create<test_obj_3_t>();
            factory.tick();
            TEST_ASSERT(obj1->ref_count() == 1);
            TEST_ASSERT(make->counter_ == 1);
            factory.collect();
            TEST_ASSERT(make->counter_ == 1);
        }
        factory.collect();

        TEST_ASSERT(make->counter_ == 0);

        return true;
    }
};

struct test_object_3_t: public test_t {

    test_object_3_t()
            : test_t("test_object_3_t") {}

    virtual bool run() override {
        using namespace tengu;

        object_factory_t factory(nullptr);
        factory.add_creator<test_obj_1_t>();

        object_ref_t ref1;
        {
            object_ref_t ref2;
            TEST_ASSERT(!ref1.valid());
            {
                object_ref_t obj = factory.create<test_obj_1_t>();
                TEST_ASSERT(obj->ref_count() == 1);
                ref1 = obj;
                TEST_ASSERT(obj->ref_count() == 2);
                {
                    ref2 = obj;
                    TEST_ASSERT(obj->ref_count() == 3);
                }
                TEST_ASSERT(obj->ref_count() == 3);
            }
            TEST_ASSERT(ref1->ref_count() == 2);
        }
        TEST_ASSERT(ref1->ref_count() == 1);
        {
            object_t *obj;
            obj = &ref1.get();
            ref1.dispose();
            TEST_ASSERT(obj->ref_count() == 0);
        }
        return true;
    }
};

struct test_object_4_t: public test_t {

    test_object_4_t()
            : test_t("test_object_4_t") {}

    virtual bool run() override {
        using namespace tengu;

        object_factory_t factory(nullptr);
        factory.add_creator<test_obj_1_t>();
        // try to create unregistered object
        object_ref_t obj1 = factory.create<test_obj_3_t>();
        TEST_ASSERT(!obj1.valid());
        // try to create registered object
        object_ref_t obj2 = factory.create<test_obj_1_t>();
        TEST_ASSERT(obj2.valid());

        return true;
    }
};

struct test_object_5_t: public test_t {

    test_object_5_t()
            : test_t("test_object_5_t") {}

    virtual bool run() override {
        using namespace tengu;

        enum {
            e_obj_test_t
        };

        struct obj_test_t : public object_ex_t<e_obj_test_t, obj_test_t> {
            int32_t *service_;
            int32_t value_;

            obj_test_t(object_service_t service)
                    : object_ex_t(), service_(static_cast<int32_t *>(service)), value_(0) {
            }

            void init(int value) {
            }
        };

        int32_t value;
        object_factory_t factory(&value);
        factory.add_creator<obj_test_t>();

        object_ref_t obj_1 = factory.create<obj_test_t>();
        TEST_ASSERT(obj_1->is_alive());
        TEST_ASSERT(obj_1->cast<obj_test_t>().service_ == &value);
        TEST_ASSERT(obj_1->cast<obj_test_t>().value_ == 0);

        object_ref_t obj_2 = factory.create<obj_test_t>(1);
        TEST_ASSERT(obj_2->is_alive());
        TEST_ASSERT(obj_2->cast<obj_test_t>().service_ == &value);
        TEST_ASSERT(obj_2->cast<obj_test_t>().value_ == 1);

        return true;
    }
};

struct test_object_6_t: public test_t {

    test_object_6_t()
            : test_t("test_object_6_t") {}

    virtual bool run() override {
        using namespace tengu;

        random_t random(0x1234);

        struct obj_t : object_ex_t<0, obj_t> {

            random_t *random_;

            obj_t(object_service_t service)
                    : object_ex_t(), random_(static_cast<random_t *>(service)) {
                order_ = random_->rand() & 0xff;
            }

            void kill() {
                if (alive_) {
                    ref_.dec();
                    alive_ = false;
                }
            }

            virtual void tick()

            override {
                if (random_->rand_chance(10)) {
                    kill();
                }
            }
        };

        object_factory_t factory(&random);
        factory.add_creator<obj_t>();

        for (int j = 0; j < 100; ++j) {
            for (int j = 0; j < 10; ++j) {
                factory.create<obj_t>();
            }
            factory.tick();
            factory.sort();
            factory.collect();
        }

        return true;
    }
};

struct test_object_7_t: public test_t {

    struct obj_t : object_ex_t<0, obj_t> {
        uint32_t id_;

        obj_t(object_service_t service)
                : object_ex_t(), id_(-1) {
        }

        void init(uint32_t id, uint32_t order) {
            id_ = id;
            order_ = order;
        }

        virtual void tick() override {
        }
    };

    test_object_7_t()
            : test_t("test_object_7_t") {}

    virtual bool run() override {
        using namespace tengu;

        uint32_t id = 0;
        object_factory_t factory(nullptr);
        factory.add_creator<obj_t>();

        factory.create<obj_t>(0, 0);
        factory.create<obj_t>(1, 1);
        factory.create<obj_t>(2, 1);
        factory.create<obj_t>(3, 1);
        factory.create<obj_t>(4, 2);

        factory.tick();

        factory.sort();
        factory.sort();
        factory.sort();
        factory.sort();

        return true;
    }
};
