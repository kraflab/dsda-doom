RSpec.describe 'sync' do
  let(:pwad) { nil }
  
  before do
    Utility.play_demo(lmp: lmp, pwad: pwad)
  end
  
  describe 'total time' do
    subject { Utility.read_levelstat.total }
    
    context 'doom2 30uv in 17:55 by Looper' do
      let(:lmp) { '30uv1755.lmp' }
      
      it { is_expected.to eq('17:55') }
    end
  end
end
