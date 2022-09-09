class Wearout_model {
    public:
        Wearout_model(void) {};
        virtual ~Wearout_model(void) {};

        /* Wearout scale function. */
        virtual double operator()(double temp) = 0;
};

