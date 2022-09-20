class Wearout_model {
   public:
    Wearout_model(void){};
    virtual Wearout_model *clone(void) = 0;
    virtual ~Wearout_model(void){};

    /* Wearout scale function. */
    virtual long double operator()(long double temp) = 0;
};

