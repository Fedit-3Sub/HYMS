classdef watertank < CsModel
   properties (Constant)
       BIG_A_VALUE = 2;         % 물탱크 단면 너비
       SMALL_A_VALUE = 0.5;     % 나가는 물의 양에 대한 상수
       MAX_HEIGHT = 0.5;        % 물탱크 최대 높이
   end
   properties
      water_height;
      is_height_under;      % 물 높이가 최대제한 수위 보다 낮은지
      file_id;
      in_water=1.0;
      under_height;
   end
   methods
        function this = watertank()
            this.VERSION = 1.0;
            this.reset();
            this.MODEL_NAME = 'watertank';
            
            this.add_inport('water_src', 0.0,'');

            this.add_outport('height', this.water_height,'');
            this.add_outport('water_over_height', int32(0),'');
            this.add_outport('water_under_height', int32(0),'');
            

            this.under_height = this.MAX_HEIGHT*0.8;
            
        end
        
        function delete(this)
            
        end

        function reset(this)
            disp('reset');
            this.water_height = 0.0;
            this.is_height_under = 1;
            this.TA = 0.1;
            this.in_water = 0.0;
        end

        function ret = set_param(this, in_param)
            ret = true;
        end

        function ret = ExtTransFn(this, sim_time, dt, in_port)
            this.in_water = in_port.value;
            
            this.set_continue();
            ret = true;
        end
        
        function ret = func(this, t, y)
            if this.water_height >= 0.0
                ret = (this.in_water - sqrt(y) * this.SMALL_A_VALUE) / this.BIG_A_VALUE;
            else
                ret = this.in_water / this.BIG_A_VALUE;
            end
        end
                
        function ret = IntTransFn(this, sim_time, dt)
            this.water_height = RK4(@this.func, this.water_height, sim_time, dt);
            ret = true;
        end 
        
        function ret = OutputFn(this, sim_time, dt)
            %fprintf(this.file_id, '%f,%f,%f\n', sim_time, this.water_height, this.in_water);
            if this.water_height > this.MAX_HEIGHT && this.is_height_under == 1
               this.is_height_under = 0;
               this.set_outport_value('water_over_height', int32(1), '');
            elseif this.water_height < this.under_height && this.is_height_under == 0
               this.is_height_under = 1;
               this.set_outport_value('water_under_height', int32(1), '');
            end
            this.set_outport_value('height', this.water_height, '');
            ret = true;
        end
        
        function ret = TimeAdvanceFn(this)
            ret = 0.1;
        end
           
   end
 
end