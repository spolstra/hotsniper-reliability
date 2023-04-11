class Rmodel {
   public:
    Rmodel(void)
        : current_R(1), current_time_stamp(0), current_sum(0),
        area_under_curve(0) {};

    Rmodel(long double sum)
        : current_R(1), current_time_stamp(0), current_sum(sum),
        area_under_curve(0) {};

    Rmodel(const Rmodel &o)
        : current_R(o.current_R),
          current_time_stamp(o.current_time_stamp),
          current_sum(o.current_sum),
          area_under_curve(o.area_under_curve) {};

    ~Rmodel(void) {};

    virtual void update(long double delta_t, long double temperature) {};

    long double get_R(void) const { return current_R; }
    long double get_sum(void) const { return current_sum; }
    long double get_area(void) const { return area_under_curve; }

   protected:
    long double current_R;
    long double current_time_stamp;
    long double current_sum;
    long double area_under_curve;
};
