RSpec.describe 'heretic', :heretic do
  let(:pwad) { nil }
  let(:iwad) { "HERETIC.WAD" }
  let(:extra) { nil }

  before do
    Utility.play_demo(lmp: lmp, iwad: iwad, pwad: pwad, extra: extra)
  end

  describe 'total time' do
    subject { Utility.read_levelstat.total }

    lines = File.readlines('spec/support/lmps/heretic/list.txt', chomp: true)
    lines.map! { |line| line.split('|') }

    lines.each do |lmp_description, lmp_time, lmp_file, extra|
      context lmp_description do
        let(:lmp) { lmp_file }
        let(:extra) { extra }

        it { is_expected.to eq(lmp_time) }
      end
    end
  end
end
