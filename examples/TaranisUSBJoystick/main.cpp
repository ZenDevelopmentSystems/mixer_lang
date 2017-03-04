
#include <cstdio>
#include "../../src/mixer.hpp"
#include "../../src/lexer.hpp"
#include "joystick.hpp"
#include <cstdio>

namespace {

   // This value simulates the airspeed sensor reading
   // TODO make a way to vary it when running ... prob
   // more suitable for a GUI version
   double airspeed_m_per_s = 0.0; //

   // true simulates a posible sensor failure 
   // which may mean airspeed reading is no good
   bool in_failsafe = false;  //

   // The mixer uses functions pointers  to get its inputs
   // The inputs can be of type Bool, Integer or Float 
   double get_airspeed(){ return airspeed_m_per_s;}

   // The input array represents all the available inputs
   // which are passed to the mixer constructor
   // TODO make inputs optionally constant
   // TODO add integer index inputs option

   apm_mix::input_pair inputs[]
    = { 
         apm_mix::input_pair{"Pitch", static_cast<double(*)()>(get_pitch)},
         apm_mix::input_pair{"Yaw",  static_cast<double(*)()>(get_yaw)},
         apm_mix::input_pair{"Roll", static_cast<double(*)()>(get_roll)},
         apm_mix::input_pair{"Throttle", static_cast<double(*)()>(get_throttle)},
         apm_mix::input_pair{"Flap", static_cast<double(*)()>(get_flap)},
         apm_mix::input_pair{"Airspeed", static_cast<double(*)()>(get_airspeed)},
         apm_mix::input_pair{"ControlMode", static_cast<double(*)()>(get_control_mode)},
         apm_mix::input_pair{"ARSPD_MIN", static_cast<double(*)()>([]()->double{return 10.0;})},
         apm_mix::input_pair{"ARSPD_CRUISE",static_cast<double(*)()>([]()->double{return 12.0;})},
         apm_mix::input_pair{"ARSPD_MAX", static_cast<double(*)()>([]()->double{return 20.0;})},
         // for completeness a bool input
         apm_mix::input_pair{"FAILSAFE_ON", static_cast<bool(*)()>([]()->bool{return in_failsafe;})},
         // for completeness an int input
         apm_mix::input_pair{"DUMMY_INT", static_cast<int64_t(*)()>([]()->int64_t{return 1000;})}
      };

   // The mixers also uses function pointers to send its outputs.
   // outputs can also be of type Bool, Integer or Float
   // Here for simplicity, we just pass an output function 'action'
   // that prints the output to stdout
   // The non type template parameter is handy to provide the index
   template<unsigned N>
   void action(double const & v)
   {
      printf("!!! output[%u] = %f\n",N,v);
   }

   // to show that bool can be assigned as an ouput function also
   template<unsigned N>
   void action(bool const & v)
   {
      const char * const true_ = "true";
      const char* const false_ = "false";
      printf("!!! output[%u] = %s\n",N, (v? true_:false_));
   }

   
   // Outputs can output a type of Integer, Bool or Float
   // Allowing different output types may be overkill
   // The types should probably be the same ( homogeneous)
   // then you can switch physical rc output channels without issues.
   // maybe though you want to set some channels true or false
   // e.g leds indicators camera switches
   // integer types can represent pwm exactly etc
   // TBD
   apm_mix::abc_expr* outputs[]
   = {
      new apm_mix::output<bool>{action<0>}
     , new apm_mix::output<double>{action<1>}
     , new apm_mix::output<double>{action<2>}
     , new apm_mix::output<double>{action<3>}
     , new apm_mix::output<double>{action<4>}
     , new apm_mix::output<double>{action<5>}
      ,new apm_mix::output<double>{action<6>}
     , new apm_mix::output<double>{action<7>}
     , new apm_mix::output<double>{action<8>}
     , new apm_mix::output<double>{action<9>}
   };
}

int main(int argc , char* argv[])
{
   if ( argc < 2){
      printf("Useage : %s <mixer_filename>\n",argv[0]);
      return EXIT_SUCCESS;
   }

   apm_mix::mixer_init(
      inputs, sizeof(inputs)/sizeof(inputs[0])
      ,outputs, sizeof(outputs)/sizeof(outputs[0])
   );

   bool mixer_build_success = false;
   if ( apm_lexer::open_file(argv[1])){
      if (apm_mix::mixer_create()){
         mixer_build_success = true;
         printf("mixer \"%s\" created OK!\n",argv[1]);
      }
      apm_lexer::close_file();
   }else{
      printf("open file %s failed\n",argv[1]);
   }
   
   if (mixer_build_success){
      printf("Press any key to start and once running press any key to quit\n");
      fflush(stdin);
      while (! key_was_pressed()){;}
      getchar(); // clear key pressed
      if ( open_joystick("/dev/input/js0")){
         while (get_joystick()->is_running() && ! key_was_pressed()){
            sleep_ms(20);
            apm_mix::mixer_eval();
         }
      }
   }
   close_joystick();
   printf("Quitting\n");
   return 0;
}

