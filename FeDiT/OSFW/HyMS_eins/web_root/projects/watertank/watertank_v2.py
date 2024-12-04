from enum import Enum
import json
from scipy import integrate as inte
import math
from CsModel import CsModel 




 
class watertank_v2(CsModel):
    def __init__(self):
        self.MODEL_NAME = "watertank"
	
        self.add_inport('water_src', 0.0)

        self.add_outport('water_over_height', 0)
        self.add_outport('water_under_height', 0)
        self.add_outport('height', 0.0)
        self.MAX_HEIGHT = 1.0        # 물탱크 최대 높이
        self.BIG_A_VALUE = 2.0       # 물탱크 단면 너비
        self.SMALL_A_VALUE = 0.5     # 나가는 물의양에 대한 상수
        #self.add_param('BIG_A_VALUE', BIG_A_VALUE, '물탱크 단면 넓이')       # 물탱크 단면 너비
        #self.add_param('SMALL_A_VALUE', SMALL_A_VALUE, '나가는물의 양')     # 나가는 물의양에 대한 상수
        #self.add_param('MAX_HEIGHT', MAX_HEIGHT, '물탱크 최대 높이')

        self.add_param('BIG_A_VALUE', self.BIG_A_VALUE, '')       # 물탱크 단면 너비
        self.add_param('SMALL_A_VALUE', self.SMALL_A_VALUE, '')     # 나가는 물의양에 대한 상수
        self.add_param('MAX_HEIGHT', self.MAX_HEIGHT, '')

        self.VERSION = 1.0
        self.reset()
        #self.log_fp = open('watertank_log_py.txt', 'w')
  
    def __del__(self):
        #self.log_fp.close()
        self.state = 1
 
    def reset(self):
        self.water_height = 0.0
        self.is_height_under = True
        self.in_water = 0.0
        self.under_height = self.MAX_HEIGHT * 0.8
        self.state = 1

    def set_param(self, port_name, port_value):
        print('set', port_name,'=', port_value)
        if port_name == "BIG_A_VALUE":
            self.BIG_A_VALUE = port_value
            print(self.BIG_A_VALUE)
        elif port_name == "SMALL_A_VALUE":
            self.SMALL_A_VALUE = port_value
            print(self.SMALL_A_VALUE)
        elif port_name == "MAX_HEIGHT":
            self.MAX_HEIGHT = port_value
            self.under_height = self.MAX_HEIGHT * 0.8
            print(self.MAX_HEIGHT)
        return True

    def rk4(self, fun, y, t, h):
        k1 = fun(t, y)
        k2 = fun(t+1/2*h, y+1/2*h*k1)
        k3 = fun(t+1/2*h, y+1/2*h*k2)
        k4 = fun(t+h, y+h*k3)

        y = y + 1/6*h*(k1 + 2*k2 + 2*k3 + k4)
        t = t + h
        return y

    def func(self, t, y):
        if(y < 0.0):
            y = 0.0
        if self.water_height >= 0.0:
            return (self.in_water - math.sqrt(y) * self.SMALL_A_VALUE) / self.BIG_A_VALUE
        else:
            return self.in_water / self.BIG_A_VALUE

    def ExtTransFn(self, sim_time, dt, src_model_name, port_name, port_value):
        self.in_water = port_value
        """
        sol = inte.RK45(self.func, sim_time, [self.water_height], sim_time + 0.1, 1)
        
        for i in range(100):
            sol.step()
            #print(i, sol.t, sol.y[0])
            if sol.status == 'finished':
                break
        self.water_height = sol.y[0]    
        """
        
        #print(self.water_height)
        self.set_continue()
        return True    

    def IntTransFn(self, sim_time, dt):
        self.water_height = self.rk4(self.func, self.water_height, sim_time, dt)
        return True

    def OutputFn(self, sim_time, dt):
        #self.log_fp.write(f'{sim_time},{self.water_height},{self.in_water}\n')
        #self.log_fp.flush()
        if self.water_height > self.MAX_HEIGHT and self.is_height_under == True:
            self.is_height_under = False
            self.set_outport_value('water_over_height', 1)
            print("Approx Tank Over Height : ", self.water_height, " at t = ", sim_time)
        elif self.water_height < self.under_height and self.is_height_under == False:
            self.is_height_under = True
            self.set_outport_value('water_under_height', 1)
            print("Approx Tank Under Height : ", self.water_height, " at t = ", sim_time)
        else:
            print("ApproxTank Height : ", self.water_height, " at t = ", sim_time)
        self.set_outport_value('height', self.water_height)
        return True

    def TimeAdvanceFn(self, sim_time):
        return 0.1


if __name__ == "__main__":
    w = watertank_v2()
    w.ExtTransFn(0.1, 'water_src', 1.0)
    print(w.MODEL_NAME, w.water_height)


   

