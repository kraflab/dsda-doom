module Utility
  extend self
  
  def play_demo(lmp:, iwad: "DOOM2.WAD", pwad: nil)
    command = "./build/prboom-plus.exe -iwad spec/support/wads/#{iwad}"
    command << " -file spec/support/wads/#{pwad}" if pwad
    command << " -fastdemo spec/support/lmps/#{lmp}"
    command << " -nosound -nomusic -nodraw -levelstat -analysis"
    
    system(command)
  end
  
  def read_analysis
    Analysis.new
  end
  
  class Analysis
    def initialize
      @data = Hash[File.readlines("analysis.txt", chomp: true).map(&:split)]
    end
    
    def pacifist?
      @data['pacifist'] == '1'
    end
    
    def reality?
      @data['reality'] == '1'
    end
    
    def almost_reality?
      @data['almost_reality'] == '1'
    end
  end
end
