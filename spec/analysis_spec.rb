RSpec.describe 'analysis' do
  let(:analysis)  { Utility.read_analysis }
  let(:pwad) { 'analysis_test.wad' }
  
  before do
    Utility.play_demo(lmp: lmp, pwad: pwad)
  end
  
  describe 'missed things' do
    let(:missed_monsters) { analysis.missed_monsters }
    let(:missed_secrets) { analysis.missed_secrets }
    
    context 'doom2 ep 3 max in 26:54 by Vile' do
      let(:lmp) { 'lve3-2654.lmp' }
      
      it 'misses one secret (map 27)' do
        expect(missed_monsters).to eq(0)
        expect(missed_secrets).to eq(1)
      end
    end
  end
  
  describe 'pacifist' do
    subject { analysis.pacifist? }
    
    context 'when there is a barrel chain' do
      let(:lmp) { 'barrel_chain.lmp' }
      
      it { is_expected.to eq(false) }
    end
    
    context 'when there is a barrel assist' do
      let(:lmp) { 'barrel_assist.lmp' }
      
      it { is_expected.to eq(false) }
    end
    
    context 'when the player shoots a keen' do
      let(:lmp) { 'keen.lmp' }
      
      it { is_expected.to eq(false) }
    end
    
    context 'when the player shoots a romero' do
      let(:lmp) { 'romero.lmp' }
      
      it { is_expected.to eq(false) }
    end
    
    context 'when there is splash damage' do
      let(:lmp) { 'splash.lmp' }
      
      it { is_expected.to eq(false) }
    end
    
    context 'when there is a telefrag' do
      let(:lmp) { 'telefrag.lmp' }
      
      it { is_expected.to eq(true) }
    end
    
    context 'when the player shoots a voodoo doll' do
      let(:lmp) { 'voodoo.lmp' }
      
      it { is_expected.to eq(true) }
    end
  end
  
  describe 'reality' do
    let(:reality) { analysis.reality? }
    let(:almost_reality) { analysis.almost_reality? }
    
    context 'when the player takes enemy damage' do
      let(:lmp) { 'damage.lmp' }
      
      it 'is not reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(false)
      end
    end
    
    context 'when the player takes nukage damage' do
      let(:lmp) { 'nukage.lmp' }
      
      it 'is almost reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(true)
      end
    end
    
    context 'when the player takes crusher damage' do
      let(:lmp) { 'crusher.lmp' }
      
      it 'is not reality' do
        expect(reality).to eq(false)
        expect(almost_reality).to eq(false)
      end
    end
    
    context 'when the player takes no damage' do
      let(:lmp) { 'reality.lmp' }
      
      it 'is reality' do
        expect(reality).to eq(true)
        expect(almost_reality).to eq(false)
      end
    end
  end
end
