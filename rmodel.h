class Rmodel {
   public:
    Rmodel(void)
        : current_R(1), current_time_stamp(0), current_state(0),
        area_under_curve(0), current_delta_v(0) {};

    Rmodel(long double state)
        : current_R(1), current_time_stamp(0), current_state(state),
        area_under_curve(0), current_delta_v(0) {};

    Rmodel(long double state, long double delta_v)
        : current_R(1), current_time_stamp(0), current_state(state),
        area_under_curve(0), current_delta_v(delta_v) {};

    Rmodel(long double timestamp, long double state, long double delta_v)
        : current_R(1), current_time_stamp(timestamp), current_state(state),
        area_under_curve(0), current_delta_v(delta_v) {};

    Rmodel(const Rmodel &o)
        : current_R(o.current_R),
          current_time_stamp(o.current_time_stamp),
          current_state(o.current_state),
          area_under_curve(o.area_under_curve),
          current_delta_v(o.current_delta_v) {};

    virtual ~Rmodel(void) = default;

    virtual void update(long double delta_t, long double temperature, long double voltage, long double stress) = 0;
    virtual void update_timestamp(long double new_time_stamp, long double temperature, long double voltage, long double stress) = 0;

    long double get_R(void) const { return current_R; }
    long double get_state(void) const { return current_state; }
    long double get_area(void) const { return area_under_curve; }
    long double get_delta_v(void) const { return current_delta_v; }

   protected:
    long double current_R;
    long double current_time_stamp;
    long double current_state;
    long double area_under_curve;
    long double current_delta_v;
};
